ftpMozilla.prototype.sftp = {
  connect : function(reconnect) {
    if (!reconnect) {                                                            // this is not a reconnection attempt
      this.isReconnecting = false;
      this.reconnectsLeft = parseInt(this.reconnectAttempts);

      if (!this.reconnectsLeft || this.reconnectsLeft < 1) {
        this.reconnectsLeft = 1;
      }
    }

    if (!this.eventQueue.length || this.eventQueue[0].cmd != "welcome") {
      this.unshiftEventQueue("welcome", "", "");                                 // wait for welcome message first
    }

    ++this.networkTimeoutID;                                                     // just in case we have timeouts from previous connection
    ++this.transferID;

    try {
      var exec = this.getExec();

      if (!exec || !exec.exists()) {
        this.onDisconnect();
        return;
      }

      this.ipcService    = Components.classes["@mozilla.org/process/ipc-service;1"].getService(Components.interfaces.nsIIPCService);
      this.pipeTransport = Components.classes["@mozilla.org/process/pipe-transport;1"].createInstance(Components.interfaces.nsIPipeTransport);
      this.ipcBuffer     = Components.classes["@mozilla.org/process/ipc-buffer;1"].createInstance(Components.interfaces.nsIIPCBuffer);

      this.ipcBuffer.open(65536, true);

      var command = exec.path.replace(/\x5c/g, "/");
      var args    = [];

      if (this.password) {
        args.push("-pw");
        args.push(this.password);
      }

      args.push("-P");
      args.push(this.port);

      if (this.useCompression) {
        args.push("-C");
      }

      if (this.privatekey) {
        args.push("-i");
        args.push(this.privatekey.replace(/\x5c/g, "/"));
      }

      args.push((this.login ? this.login + "@" : "") + this.host);

      this.pipeTransport.init(command, args, args.length, [], 0, 0, "", true, true, this.ipcBuffer);

      this.controlOutstream = this.pipeTransport.openOutputStream(0, 0, 0);

      var self = this;
      var dataListener;
      var func = function() {
        if (!self.pipeTransport.isAttached()) {
          self.onDisconnect();
        }

        if (dataListener.data) {
          if (dataListener.data.indexOf('\npsftp>') == -1 && dataListener.data.indexOf('\nStore key in cache') == -1
           && dataListener.data.indexOf('\nUpdate cached key') == -1
           && dataListener.data.indexOf('\nAccess denied') == -1
           && dataListener.data.indexOf('Fatal: Network error:') == -1
           && dataListener.data.indexOf('ssh_init:') != 0) {
            return;
          }

          var buf = dataListener.data;
          dataListener.data = "";
          self.readControl(buf);
        }
      };
      this.readPoller = setInterval(func, 100);

      dataListener = {
        data            : "",

        onStartRequest  : function(request, context) { },

        onStopRequest   : function(request, context, status) { },

        onDataAvailable : function(request, context, inputStream, offset, count) {
          var controlInstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
          controlInstream.init(inputStream);
          this.data += controlInstream.read(count);
        }
      };

      this.pipeTransport.asyncRead(dataListener, null, 0, 0, 0);

    } catch(ex) {
      this.onDisconnect();
    }
  },

  keepAlive : function() {
    // do nothing
  },

  readControl : function(buffer) {
    if (this.isKilling) {
      return;
    }

    try {
      buffer = this.toUTF8.convertStringToUTF8(buffer, this.encoding, 1);
    } catch (ex) {
      if (this.observer) {
        this.observer.onDebug(ex);
      }
    }

    if ((buffer == "2" && !this.isConnected) || buffer == "\r\n" || buffer == "\n") {
      return;
    }

    buffer         = buffer.replace(/\r\npsftp> /, '');
    buffer         = buffer.replace(/\npsftp> /,   '');
    var origBuffer = buffer;
    buffer         = buffer.indexOf("\r\n") != -1 ? buffer.split("\r\n") : buffer.split("\n");
    buffer         = buffer.filter(this.removeBlanks);

    if (origBuffer != "2" && origBuffer.indexOf('Store key in cache') == -1      // "2"s are self-generated fake messages
     && origBuffer.indexOf('\nUpdate cached key') == -1
     && origBuffer.indexOf('Fatal: Network error:') == -1
     && origBuffer.indexOf('\nAccess denied') == -1
     && origBuffer.indexOf('ssh_init:') != 0) {
      for (var x = 0; x < buffer.length; ++x) {                                  // add response to log
        var message   = buffer[x].charAt(buffer[x].length - 1) == '\r'
                      ? buffer[x].substring(0, buffer[x].length - 1) : buffer[x];
        if (this.observer) {
          this.observer.onAppendLog(message, 'input', "info");
        }

        if (message.indexOf('Listing directory') != -1) {
          break;
        }
      }

      ++this.networkTimeoutID;
    }

    var cmd;  var parameter;    var callback;   var callback2;

    if (this.eventQueue.length) {
      cmd        = this.eventQueue[0].cmd;
      parameter  = this.eventQueue[0].parameter;
      callback   = this.eventQueue[0].callback;
      callback2  = this.eventQueue[0].callback2;

      if (cmd != "ls" && cmd != "get" && cmd != "reget" && cmd != "put" && cmd != "reput") {   // used if we have a loss in connection
        var throwAway = this.eventQueue.shift();

        if (throwAway.cmd != "welcome" && throwAway.cmd != "goodbye" && throwAway.cmd != "aborted" && throwAway.cmd != "sftpcache") {
          this.trashQueue.push(throwAway);
        }
      }
    } else {
      cmd = "default";                                                           // an unexpected reply - perhaps a 421 timeout message
    }

    switch (cmd) {
      case "welcome":
      case "sftpcache":
        if (origBuffer.indexOf('Fatal: Network error:') != -1 || origBuffer.indexOf('ssh_init:') == 0) {
          this.legitClose = true;
          this.onDisconnect();
          return;
        }

        if (origBuffer.indexOf('Store key in cache') != -1 || origBuffer.indexOf('\nUpdate cached key') != -1) {
          if (this.observer) {
            var answer = this.observer.onSftpCache(origBuffer);

            if (answer) {
              this.unshiftEventQueue("sftpcache", answer, "");
            } else {
              this.legitClose = true;
              this.onDisconnect();
              return;
            }

            break;
          }
        }

        this.isConnected       = true;                                           // good to go

        if (this.observer) {
          this.observer.onConnected();
        }

        this.isReconnecting    = false;
        this.reconnectsLeft    = parseInt(this.reconnectAttempts);               // setup reconnection settings

        if (!this.reconnectsLeft || this.reconnectsLeft < 1) {
          this.reconnectsLeft = 1;
        }

        var newConnectedHost = this.login + "@" + this.host;

        if (this.observer) {
          this.observer.onLoginAccepted(newConnectedHost != this.connectedHost);
        }

        if (newConnectedHost != this.connectedHost) {
          this.legitClose = true;
        }

        this.connectedHost = newConnectedHost;                                   // switching to a different host or different login

        if (!this.legitClose) {
          this.recoverFromDisaster();                                            // recover from previous disaster
          break;
        }

        this.legitClose   = false;

        origBuffer = origBuffer.substring(origBuffer.indexOf("Remote working directory is") + 28);  // if buffer is not '/' we're chrooted
        this.currentWorkingDir = origBuffer;

        if (this.observer) {
          this.observer.onChangeDir(origBuffer != '/' && this.initialPath == '' ? origBuffer : '', false, origBuffer != '/' || this.initialPath != '');
        }

        this.trashQueue = new Array();

        break;

      case "ls":
      case "get":
      case "reget":
      case "put":
      case "reput":
        ++this.transferID;
        this.eventQueue.shift();
        if (this.eventQueue.length && this.eventQueue[0].cmd == "transferEnd") {
          this.eventQueue.shift();
        }
        this.trashQueue = new Array();                                       // clear the trash array, completed an 'atomic' set of operations

        if (cmd == "ls") {
          this.listData = this.parseListData(origBuffer, parameter);

          if (typeof callback == "string") {
            eval(callback);                                                  // send off list data to whoever wanted it
          } else {
            callback();
          }
        }

        if (callback2) {                                                     // for transfers
          if (typeof callback2 == "string") {
            eval(callback2);
          } else {
            callback2();
          }
        }

        break;

      case "mkdir":
      case "rm":
      case "rmdir":
        if (origBuffer.indexOf(': OK') != origBuffer.length - 4) {
          if (this.observer) {
            this.observer.onError(origBuffer + ": " + this.constructPath(this.currentWorkingDir, parameter));
          }
        } else {
          if (cmd == "rmdir") {                                                  // clear out of cache if it's a remove directory
            this.removeCacheEntry(this.constructPath(this.currentWorkingDir, parameter));
          }

          if (typeof callback == "string") {
            eval(callback);                                                      // send off list data to whoever wanted it
          } else {
            callback();
          }
        }

        this.trashQueue = new Array();
        break;

      case "mv":
        if (origBuffer.indexOf(': no such file or directory') != -1) {
          if (this.observer) {
            this.observer.onError(origBuffer + ": " + parameter);
          }
        } else {
          if (typeof callback == "string") {
            eval(callback);                                                      // send off list data to whoever wanted it
          } else {
            callback();
          }
        }

        this.trashQueue = new Array();
        break;

      case "chmod":
        if (typeof callback == "string") {
          eval(callback);                                                        // send off list data to whoever wanted it
        } else {
          callback();
        }

        this.trashQueue = new Array();
        break;

      case "cd":
        if (origBuffer.indexOf('Remote directory is now') == -1) {               // if it's not a directory
          if (callback && typeof callback == "function") {
            callback(false);
          } else if (this.observer) {
            this.observer.onDirNotFound(origBuffer);

            if (this.observer) {
              this.observer.onError(origBuffer);
            }
          }
        } else {
          this.currentWorkingDir = parameter;

          if (this.observer) {                                                   // else navigate to the directory
            this.observer.onChangeDir(parameter, typeof callback == "boolean" ? callback : "");
          }

          if (callback && typeof callback == "function") {
            callback(true);
          }
        }
        break;

      case "aborted":
      case "custom":
        break;

      case "goodbye":                                                            // you say yes, i say no, you stay stop...
      default:
        if (origBuffer.indexOf('Access denied') != -1) {
          if (this.observer && this.type == 'transfer') {
            this.observer.onLoginDenied();
          }

          this.cleanup();                                                        // login failed, cleanup variables

          if (this.observer && this.type != 'transfer' && this.type != 'bad') {
            this.observer.onError(origBuffer);
          }

          this.isConnected = false;

          this.kill();

          if (this.type == 'transfer') {
            this.type = 'bad';
          }

          if (this.observer && this.type != 'transfer' && this.type != 'bad') {
            var self = this;
            var func = function() { self.observer.onLoginDenied(); };
            setTimeout(func, 0);
          }

          return;
        } else if (this.observer) {
          this.observer.onError(origBuffer);
        }
        break;
    }

    this.isReady = true;

    if (this.observer) {
      this.observer.onIsReadyChange(true);
    }

    if (this.eventQueue.length && this.eventQueue[0].cmd != "welcome") {         // start the next command
      this.writeControl();
    } else {
      this.refresh();
    }
  },

  changeWorkingDirectory : function(path, callback) {
    this.addEventQueue("cd", path, callback);
    this.writeControlWrapper();
  },

  makeDirectory : function(path, callback) {
    this.addEventQueue("cd",    path.substring(0, path.lastIndexOf('/') ? path.lastIndexOf('/') : 1), true);
    this.addEventQueue("mkdir", path.substring(path.lastIndexOf('/') + 1), callback);
    this.writeControlWrapper();
  },

  makeBlankFile : function(path, callback) {
    this.addEventQueue("cd", path.substring(0, path.lastIndexOf('/') ? path.lastIndexOf('/') : 1), true);

    try {
      var count = 0;
      let tmpFile = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties).get("TmpD", Components.interfaces.nsILocalFile);
      tmpFile.append(count + '-blankFile');
      while (tmpFile.exists()) {
        ++count;
        tmpFile.leafName = count + '-blankFile';
      }
      var foutstream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
      foutstream.init(tmpFile, 0x04 | 0x08 | 0x20, 0644, 0);
      foutstream.write("", 0);
      foutstream.close();

      this.upload(tmpFile.path, path, false, 0, 0, callback, true);
    } catch (ex) {
      if (this.observer) {
        this.observer.onDebug(ex);
      }
    }
  },

  remove : function(isDirectory, path, callback) {
    if (isDirectory) {
      this.unshiftEventQueue("rmdir", path.substring(path.lastIndexOf('/') + 1).replace(/\[/g, "\\[").replace(/\]/g, "\\]"), callback);
      this.unshiftEventQueue("cd",    path.substring(0, path.lastIndexOf('/') ? path.lastIndexOf('/') : 1), true);

      var self         = this;
      var listCallback = function() { self.removeRecursive(path); };
      this.list(path, listCallback, true, true);
    } else {
      this.unshiftEventQueue("rm",    path.substring(path.lastIndexOf('/') + 1).replace(/\[/g, "\\[").replace(/\]/g, "\\]"), callback);
      this.unshiftEventQueue("cd",    path.substring(0, path.lastIndexOf('/') ? path.lastIndexOf('/') : 1), true);
    }

    this.writeControlWrapper();
  },

  removeRecursive : function(parent) {                                           // delete subdirectories and files
    var files = this.listData;

    for (var x = 0; x < files.length; ++x) {
      var remotePath = this.constructPath(parent, files[x].leafName);

      if (files[x].isDirectory()) {                                              // delete a subdirectory recursively
        this.unshiftEventQueue("rmdir",  remotePath.substring(remotePath.lastIndexOf('/') + 1).replace(/\[/g, "\\[").replace(/\]/g, "\\]"), "");
        this.unshiftEventQueue("cd",     parent, true);
        this.removeRecursiveHelper(remotePath);
      } else {                                                                   // delete a file
        this.unshiftEventQueue("rm",     remotePath.substring(remotePath.lastIndexOf('/') + 1).replace(/\[/g, "\\[").replace(/\]/g, "\\]"), "");
        this.unshiftEventQueue("cd",     parent, true);
      }
    }
  },

  removeRecursiveHelper : function(remotePath) {
    var self           = this;
    var listCallback   = function() { self.removeRecursive(remotePath); };
    this.list(remotePath, listCallback, true, true);
  },

  rename : function(oldName, newName, callback, isDir) {
    if (isDir) {
      this.removeCacheEntry(oldName);
    }

    oldName = oldName.replace(/\[/g, "\\[").replace(/\]/g, "\\]");

    this.addEventQueue("mv", '"' + this.escapeSftp(oldName) + '" "' + this.escapeSftp(newName) + '"', callback);                 // rename the file
    this.writeControlWrapper();
  },

  changePermissions : function(permissions, path, callback) {
    path = path.replace(/\[/g, "\\[").replace(/\]/g, "\\]");

    this.addEventQueue("cd",    path.substring(0, path.lastIndexOf('/') ? path.lastIndexOf('/') : 1), true);
    this.addEventQueue("chmod", permissions + ' "' + this.escapeSftp(path.substring(path.lastIndexOf('/') + 1)) + '"', callback);
    this.writeControlWrapper();
  },

  custom : function(cmd) {
    this.addEventQueue("custom", cmd);
    this.writeControlWrapper();
  },

  list : function(path, callback, skipCache, recursive, fxp) {
    if (!skipCache && this.sessionsMode) {
      if (this.cacheHit(path, callback)) {
        return;
      }
    }

    if (recursive) {
      this.unshiftEventQueue(  "ls", path, callback, '');
      this.unshiftEventQueue(  "cd", path, "",       '');
    } else {
      this.addEventQueue(      "cd", path, "",       '');
      this.addEventQueue(      "ls", path, callback, '');
    }

    this.writeControlWrapper();
  },

  download : function(remotePath, localPath, remoteSize, resume, localSize, isSymlink, callback) {
    ++this.queueID;
    var id = this.connNo + "-" + this.queueID;

    this.addEventQueue("transferBegin", "", { id: id });

    this.addEventQueue(  "cd",  remotePath.substring(0, remotePath.lastIndexOf('/') ? remotePath.lastIndexOf('/') : 1), true);

    var leafName = remotePath.substring(remotePath.lastIndexOf('/') + 1);

    this.addEventQueue(resume ? "reget" : "get", '"' + this.escapeSftp(leafName) + '" "' + this.escapeSftp(localPath.replace(/\x5c/g, "/")) + '"', localPath, callback);

    this.addEventQueue("transferEnd", "", { localPath: localPath, remotePath: remotePath, size: remoteSize, transport: 'sftp', type: 'download', ascii: "I", id: id });

    this.writeControlWrapper();
  },

  upload : function(localPath, remotePath, resume, localSize, remoteSize, callback, disableMDTM) {
    ++this.queueID;
    var id = this.connNo + "-" + this.queueID;

    this.addEventQueue("transferBegin", "", { id: id });

    this.addEventQueue(  "cd",  remotePath.substring(0, remotePath.lastIndexOf('/') ? remotePath.lastIndexOf('/') : 1), true);

    var leafName = remotePath.substring(remotePath.lastIndexOf('/') + 1);

    this.addEventQueue(resume ? "reput" : "put", '"' + this.escapeSftp(localPath.replace(/\x5c/g, "/")) + '" "' + this.escapeSftp(leafName) + '"', localPath, callback);

    this.addEventQueue("transferEnd", "", { localPath: localPath, remotePath: remotePath, size: localSize, transport: 'sftp', type: 'upload', ascii: "I", id: id });

    this.writeControlWrapper();

    return id;
  },

  isListing : function() {                                                       // check queue to see if we're listing
    for (var x = 0; x < this.eventQueue.length; ++x) {
      if (this.eventQueue[x].cmd.indexOf("ls") != -1) {
        return true;
      }
    }

    return false;
  },

  recoverFromDisaster : function() {                                             // after connection lost, try to restart queue
    if (this.eventQueue.length && this.eventQueue[0].cmd == "goodbye") {
      this.eventQueue.shift();
    }

    if (this.eventQueue.cmd) {
      this.eventQueue = new Array(this.eventQueue);
    }

    if (this.eventQueue.length && (this.eventQueue[0].cmd == "ls"
                               ||  this.eventQueue[0].cmd == "get"
                               ||  this.eventQueue[0].cmd == "reget"
                               ||  this.eventQueue[0].cmd == "put"
                               ||  this.eventQueue[0].cmd == "reput")) {
      var cmd       = this.eventQueue[0].cmd;
      var parameter = this.eventQueue[0].parameter;

      cmd = this.eventQueue[0].cmd;

      if (cmd == "put") {                                                        // set up resuming for these poor interrupted transfers
        this.eventQueue[0].cmd = "reput";
      } else if (cmd == "get") {
        this.eventQueue[0].cmd = "reget";
      }
    }

    if (this.currentWorkingDir) {
      this.unshiftEventQueue("cd", this.currentWorkingDir, true);
      this.currentWorkingDir = "";
    }

    this.trashQueue = new Array();
  },

  getExec : function() {
    if (this.getPlatform() == "windows") {
      var exec = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties)
                           .get("ProfD", Components.interfaces.nsILocalFile);
      exec.append("extensions");
      exec.append("{a7c6cf7f-112c-4500-a7ea-39801a327e5f}");
      exec.append("platform");
      exec.append("WINNT_x86-msvc");
      exec.append("psftp.exe");

      return exec;
    } else if (this.getPlatform() == "linux") {
      var file = Components.classes['@mozilla.org/file/local;1'].createInstance(Components.interfaces.nsILocalFile);
      file.initWithPath("/usr/bin/psftp");

      return file;
    } else if (this.getPlatform() == "mac") {
      var file = Components.classes['@mozilla.org/file/local;1'].createInstance(Components.interfaces.nsILocalFile);
      file.initWithPath("/opt/local/var/macports/software/putty");

      if (!file.exists()) {
        return file;
      }

      var subdirs = [];

      var entries = file.directoryEntries;                                       // find highest version number
      while (entries.hasMoreElements()) {
        subdirs.push(entries.getNext().QueryInterface(Components.interfaces.nsILocalFile));
      }

      subdirs.sort(compareName);
      subdirs.reverse();

      if (!subdirs.length) {
        return file;
      }

      file.append(subdirs[0].leafName);
      file.append("opt");
      file.append("local");
      file.append("bin");
      file.append("psftp");

      return file;
    }
  },

  getPlatform : function() {
    var platform = navigator.platform.toLowerCase();

    if (platform.indexOf('linux') != -1) {
      return 'linux';
    }

    if (platform.indexOf('mac') != -1) {
      return 'mac';
    }

    return 'windows';
  }
};
