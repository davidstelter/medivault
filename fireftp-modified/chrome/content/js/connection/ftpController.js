function connect(noAccountChange, showPassDialog) {
  if (!noAccountChange) {
    onAccountChange();
  }

  gStatusBarClear = false;

  for (var x = 0; x < gMaxCon; ++x) {
    gConnections[x].host = gConnections[x].host.replace(/^ftp:\/*/, '');    // error checking - get rid of 'ftp://'

    if (gConnections[x].host && gConnections[x].host.charAt(gConnections[x].host.length - 1) == '/') {
      gConnections[x].host = gConnections[x].host.substring(0, gConnections[x].host.length - 1);
    }
  }

  if (gFtp.host == "about:mozilla") {                                       // just for fun
    window.openDialog("chrome://fireftp/content/welcome.xul", "welcome", "chrome,dialog,resizable,centerscreen", "", true);
    gConnectButton.label = "Flame On!";
    gConnectButton.setAttribute('accesskey', "F");
    return;
  }

  if (!gFtp.host) {                                                         // need to fill in the host
    doAlert(gStrbundle.getString("alertFillHost"));
    return;
  }

  if (!gFtp.port || !parseInt(gFtp.port)) {                                 // need a valid port
    doAlert(gStrbundle.getString("alertFillPort"));
    return;
  }

  if (!gFtp.login || !gFtp.password || showPassDialog) {                    // get a password if needed
    var passwordObject       = new Object();
    passwordObject.login     = gFtp.login;
    passwordObject.password  = gFtp.password;
    passwordObject.returnVal = false;

    window.openDialog("chrome://fireftp/content/password.xul", "password", "chrome,modal,dialog,resizable,centerscreen", passwordObject);

    if (passwordObject.returnVal) {
      for (var x = 0; x < gMaxCon; ++x) {
        gConnections[x].login    = passwordObject.login;
        gConnections[x].password = passwordObject.password;
      }
    } else {
      return;
    }
  }

  if (gFtp.security == "sftp" && gPlatform == 'linux') {
    if (!gFtp.getExec().exists()) {
      doAlert(gStrbundle.getString("sftpLinuxNotFound"));
      return;
    }
  }

  if (gFtp.security == "sftp" && gPlatform == 'mac') {
    if (!gFtp.getExec().exists()) {
      doAlert(gStrbundle.getString("sftpMacNotFound"));
      return;
    }
  }

  if (gFtp.security == "sftp" && gFtp.privatekey) {
    var pk = localFile.init(gFtp.privatekey);

    if (!pk || !pk.exists()) {
      doAlert(gStrbundle.getString("pkNotFound"));
      return;
    }
  }

  setConnectButton(false);

  for (var x = 0; x < gSiteManager.length; ++x) {                           // print out debug info; help me help you
    if (gSiteManager[x].account == gAccount) {
      var debugSite = new cloneObject(gSiteManager[x]);
      debugSite.password = "";
      debug(debugSite.toSource(), "DEBUG");
      break;
    }
  }

  debug(  "gConcurrent:"    + gConcurrent
      + ", gMaxCon:"        + gMaxCon
      + ", gRefreshMode:"   + gRefreshMode
      + ", gTempPasvMode:"  + gTempPasvMode
      + ", gLoadUrl:"       + (gLoadUrl ? 'true' : 'false')
      + ", fileMode:"       + gFtp.fileMode
      + ", hiddenMode:"     + gFtp.hiddenMode
      + ", keepAliveMode:"  + gFtp.keepAliveMode
      + ", networkTimeout:" + gFtp.networkTimeout
      + ", proxyHost:"      + gFtp.proxyHost
      + ", proxyPort:"      + gFtp.proxyPort
      + ", proxyType:"      + gFtp.proxyType
      + ", activePortMode:" + gFtp.activePortMode
      + ", activeLow:"      + gFtp.activeLow
      + ", activeHigh:"     + gFtp.activeHigh
      + ", reconnectMode:"  + gFtp.reconnectMode
      + ", sessionsMode:"   + gFtp.sessionsMode
      + ", timestampsMode:" + gFtp.timestampsMode
      + ", useCompression:" + gFtp.useCompression
      + ", integrityMode:"  + gFtp.integrityMode
      + ", userAgent:"      + navigator.userAgent, "DEBUG");

  gFtp.connect();
}

function disconnect() {
  var working = false;

  for (var x = 0; x < gMaxCon; ++x) {
    if (gConnections[x].isConnected && gConnections[x].eventQueue.length && (gConnections[x].eventQueue.length > 1
                                                                         || (gConnections[x].eventQueue[0].cmd != "NOOP" && gConnections[x].eventQueue[0].cmd != "aborted"))) {
      working = true;
      break;
    }
  }

  if (working && !confirm(gStrbundle.getString("reallyclose"))) {
    return;
  }

  setConnectButton(true);
  gRemotePath.value = '/';
  gRemotePathFocus  = '/';
  document.title    = "FireFTP";

  for (var x = 0; x < gMaxCon; ++x) {
    if (gConnections[x].isConnected) {
      gConnections[x].disconnect();
    }

    if (gFtp.security == "sftp") {
      gConnections[x].sftpKill();
    }
  }

  if (gFxp && gFxp.isConnected) {
    gFxp.disconnect();
  }
}

var securityCallbacks = {
  connection : null,

  getInterface : function(iid, instance) {
    if (iid.equals(Components.interfaces.nsIBadCertListener2)) {
      return this;
    }

    return null;
  },

  notifyCertProblem : function(socketInfo, status, targetSite) {
    var self = this;

    var func = function() {
      var flags = gPromptService.BUTTON_TITLE_IS_STRING * gPromptService.BUTTON_POS_0 +
                  gPromptService.BUTTON_TITLE_IS_STRING * gPromptService.BUTTON_POS_1;
      var response = gPromptService.confirmEx(window, gStrbundle.getString("secureConnFailed"),
                                                     gStrbundle.getFormattedString("usesInvalidCert", [self.connection.host]) + "\n\n"
                                                   + (status.isDomainMismatch     ? gStrbundle.getFormattedString("domainMismatch", [status.serverCert.commonName]) + "\n" : "")
                                                   + (status.isNotValidAtThisTime ? gStrbundle.getString("isNotValidAtThisTime") + "\n" : "")
                                                   + (status.isUntrusted          ? gStrbundle.getString("isUntrusted") + "\n" : "")
                                                   + "\n" + gStrbundle.getString("secureConnFailedDesc"), flags,
                                                     gStrbundle.getString("cancelButton"),
                                                     gStrbundle.getString("orSeeWhatsBehindDoorNumberTwo"),
                                                     null, null, {});

      if (response == 0) {
        self.connection.onDisconnect();
        self.connection = null;
        return;
      }

      response = gPromptService.confirmEx(window, gStrbundle.getString("secureConnFailed"),
                                                 gStrbundle.getString("addExceptionDesc"), flags,
                                                 gStrbundle.getString("addExceptionEscape"),
                                                 gStrbundle.getString("addException"),
                                                 null, null, {});

      if (response == 0) {
        self.connection.onDisconnect();
        self.connection = null;
        return;
      }

      var params = { location : targetSite, exceptionAdded : false };
      window.openDialog('chrome://pippki/content/exceptionDialog.xul', '', 'chrome,centerscreen,modal', params);

      if (params.exceptionAdded) {
        self.connection.onDisconnect(true);
        self.connection.connect();
      } else {
        self.connection.onDisconnect();
      }

      self.connection = null;
    };

    setTimeout(func, 0);

    return true;
  }
};

function securityPopup(secure) {
  $('identity-popup-container').className              = secure ? "verifiedDomain" : "unknownIdentity";
  $('identity-popup-content-box').className            = secure ? "verifiedDomain" : "unknownIdentity";
  $('identity-popup-content').textContent              = secure ? gFtp.host                           : "";
  $('identity-popup-content-supplemental').textContent = secure ? gStrbundle.getString("locVerified") : gStrbundle.getString("idUnknown");
  $('identity-popup-content-verifier').textContent     = secure ? ""                                  : "";
  $('identity-popup-encryption-label').textContent     = secure ? gStrbundle.getString("encrypted")   : gStrbundle.getString("notEncrypted");
}

var ftpObserver = {
  extraCallback     : null,
  securityCallbacks : securityCallbacks,

  onConnectionRefused : function() {
    displayWelcomeMessage(gFtp.welcomeMessage);
    setConnectButton(true);
  },

  onConnected : function() {
    connectedButtonsDisabler();
    setConnectButton(false);

    if (gFtp.security) {
      $('remotepath').setAttribute("security", "on");
      securityPopup(true);
    }

    for (var x = 1; x < gMaxCon; ++x) {
      gConnections[x].type = 'transfer';
    }

    $('page-proxy-favicon').src = (gWebHost ? gWebHost : "http://" + gFtp.host) + "/favicon.ico";
  },

  onWelcomed : function() {
    displayWelcomeMessage(gFtp.welcomeMessage);
  },

  onLoginAccepted : function(newHost) {
    if (gFtp.isConnected && newHost) {                                      // switching to a different host or different login
      remoteTree.treebox.rowCountChanged(0,    -remoteTree.rowCount);
      remoteTree.rowCount    = 0;
      remoteTree.data        = new Array();
      remoteDirTree.treebox.rowCountChanged(0, -remoteDirTree.rowCount);
      remoteDirTree.rowCount = 0;
      remoteDirTree.data     = new Array();
    }
  },

  onLoginDenied : function() {
    connect(false, true);
  },

  onDisconnected : function(attemptingReconnect) {
    try {
      if (connectedButtonsDisabler) {                                       // connectedButtonsDisabler could be gone b/c we're disposing
        connectedButtonsDisabler();
        setConnectButton(true);
        remoteDirTree.extraCallback = null;
        remoteTree.extraCallback    = null;
        remoteTree.errorCallback    = null;
        this.extraCallback          = null;
        gTreeSyncManager            = false;
        remoteTree.pasteFiles       = new Array();
        document.title              = "FireFTP";
        $('remotePasteContext').setAttribute("disabled", true);
        $('remotepath').removeAttribute("security");
        securityPopup();

        if (gFxp && gFxp.isConnected) {
          gFxp.disconnect();
        }

        $('page-proxy-deck').selectedIndex = 0;
        $('page-proxy-favicon').src = "";

        if (!attemptingReconnect) {
          for (var x = 0; x < gTempEditFiles.length; ++x) {
            gFireFTPUtils.removeFile(gTempEditFiles[x].file);
            clearInterval(gTempEditFiles[x].id);
          }

          gTempEditFiles = [];
        }
      }
    } catch (ex) { }
  },

  onReconnecting : function() {
    $('abortbutton').disabled = false;
  },

  onAbort : function() {
    remoteDirTree.extraCallback = null;
    remoteTree.extraCallback    = null;
    remoteTree.errorCallback    = null;
    this.extraCallback          = null;
    gTreeSyncManager            = false;

    if (!gSearchRunning) {
      localTree.refresh();
      remoteTree.refresh();
    }

    if (gFxp && gFxp.isConnected) {
      gFxp.disconnect();
    }
  },

  onError : function(msg) {
    error(msg, false, true);

    remoteDirTree.extraCallback = null;
    remoteTree.extraCallback    = null;
    this.extraCallback          = null;

    if (remoteTree.errorCallback) {
      var tempCallback          = remoteTree.errorCallback;
      remoteTree.errorCallback  = null;
      tempCallback();
    }

    if (gFxp && gFxp.isConnected) {
      gFxp.disconnect();
    }
  },

  onDebug : function(msg, level) {
    debug(msg, level, true);
  },

  onAppendLog : function(msg, css, type) {
    appendLog(msg, css, type, true);
  },

  onIsReadyChange : function(state) {
    try {
      window.onbeforeunload = state ? null : beforeUnload;

      if (gLoadUrl && state && gFtp.isConnected && !gFtp.eventQueue.length) { // if it's an external link check to see if it's a file to download
        var leafName = gLoadUrl.substring(gLoadUrl.lastIndexOf('/') + 1);
        var index = -1;

        for (var x = 0; x < gFtp.listData.length; ++x) {
          if (leafName == gFtp.listData[x].leafName) {
            index = x;
            break;
          }
        }

        var loadUrl = gLoadUrl;
        gLoadUrl    = "";

        if (index == -1) {
          appendLog(gStrbundle.getString("remoteNoExist"), 'error', "error");
          return;
        }

        if (gFtp.listData[index].isDirectory()) {
          remoteDirTree.changeDir(loadUrl);
        } else {                                                              // if it is, well, then download it
          var prefBranch = gPrefsService.getBranch("browser.");

          try {
            if (!prefBranch.getBoolPref("download.useDownloadDir")) {
              if (!browseLocal(gStrbundle.getString("saveFileIn"))) {
                return;
              }
            }
          } catch (ex) { }

          remoteTree.selection.select(index);
          new transfer().start(true);
        }
      }
    } catch (ex) { }
  },

  onShouldRefresh : function(local, remote, dir) {
    for (var x = 0; x < gMaxCon; ++x) {
      if (!gConnections[x].isConnected) {
        continue;
      }

      if (gConnections[x].eventQueue.length && gConnections[x].eventQueue[0].cmd != "welcome") {
        if (local) {
          gConnections[x].localRefreshLater = dir;
        }

        if (remote) {
          gConnections[x].remoteRefreshLater = dir;
        }
        return;
      }
    }

    if (gRefreshMode && local) {
      if (this.extraCallback) {
        var tempCallback   = this.extraCallback;
        this.extraCallback = null;
        tempCallback();
      } else {
        if (gLocalPath.value != dir) {
          localDirTree.addDirtyList(dir);
        } else {
          localTree.refresh();
        }
      }
    }

    if (gRefreshMode && remote) {
      if (this.extraCallback) {
        var tempCallback   = this.extraCallback;
        this.extraCallback = null;
        tempCallback();
      } else {
        if (gRemotePath.value != dir) {
          remoteDirTree.addDirtyList(dir);
        } else {
          remoteTree.refresh();
        }
      }
    }
  },

  onChangeDir : function(path, dontUpdateView, skipRecursion) {
    if (!dontUpdateView) {
      if (skipRecursion) {
        gRemotePath.value = path ? path : gRemotePath.value;
        remoteDirTree.dontPanic();                                          // don't forget to bring a towel
      } else {
        remoteDirTree.changeDir(path ? path : gRemotePath.value);
      }
    }
  },

  onDirNotFound : function(buffer) {                                        // so this isn't exactly the cleanest way to do it, bite me
    var changeDirPath;

    if (gFtp.eventQueue.length > 1 && gFtp.eventQueue[1].cmd == "LIST" && (typeof gFtp.eventQueue[1].callback == "string")
                                                                       && gFtp.eventQueue[1].callback.indexOf("remoteDirTree.changeDir(") != -1) {
      changeDirPath = gFtp.eventQueue[1].callback.substring(gFtp.eventQueue[1].callback.indexOf("'") + 1, gFtp.eventQueue[1].callback.length - 2);
    }

    if (gFtp.eventQueue.length > 1 && gFtp.eventQueue[1].cmd == "LIST") {
      gFtp.eventQueue.shift();                                              // get rid of pasv and list in the queue
      gFtp.eventQueue.shift();
      gFtp.trashQueue = new Array();
    }

    if (changeDirPath) {                                                    // this is a fix for people who can't access '/' on their remote hosts
      gRemotePath.value = changeDirPath;
      remoteDirTree.dontPanic();                                            // don't forget to bring a towel
    }
  },

  onTransferFail : function(params, reason) {
    queueTree.addFailed(params, reason);
  },

  onSftpCache : function(buffer) {
    var key = buffer.indexOf("\r\n") != -1 ? buffer.split("\r\n") : buffer.split("\n");
    var index = 4;

    for (var x = 0; x < key.length; ++x) {
      if (key[x].indexOf('is:') != -1) {
        index = x + 1;
        break;
      }
    }

    key = key[index];

    var flags    = gPromptService.BUTTON_TITLE_YES    * gPromptService.BUTTON_POS_0 +
                   gPromptService.BUTTON_TITLE_NO     * gPromptService.BUTTON_POS_2 +
                   gPromptService.BUTTON_TITLE_CANCEL * gPromptService.BUTTON_POS_1;
    var response = gPromptService.confirmEx(window, gStrbundle.getString("sftpCacheTitle"),
                                                    gStrbundle.getFormattedString("sftpCache", [key]), flags,
                                                    null, null, null, null, {});
    return response == 0 ? 'y' : (response == 2 ? 'n' : '');
  }
};

function transferObserver(connNo) {
  this.connNo = connNo;
}

transferObserver.prototype = {
  securityCallbacks   : null,

  transferQueue       : function() {
    var foundTransfer = false;

    for (var x = 0; x < gConnections[this.connNo - 1].eventQueue.length; ++x) {
      if (foundTransfer || gConnections[this.connNo - 1].eventQueue[x].cmd == "transferBegin") {
        foundTransfer = true;
        gFtp.eventQueue.push(gConnections[this.connNo - 1].eventQueue[x]);
      }
    }
  },

  onConnectionRefused : function()                   {
    this.transferQueue();
  },
  onWelcomed          : function()                   { },
  onConnected         : function()                   { },
  onLoginAccepted     : function(newHost)            { },
  onLoginDenied       : function()                   {
    this.transferQueue();
  },
  onDisconnected      : function()                   { },
  onReconnecting      : function()                   { },
  onAbort             : function()                   { },
  onError             : function(msg)                { error("[" + this.connNo + "] " + msg, false, true); },
  onDebug             : function(msg, level)         { debug("[" + this.connNo + "] " + msg, level, true); },
  onAppendLog         : function(msg, css, type)     { appendLog((gDebugMode ? "[" + this.connNo + "] " : "") + msg, css, type, true); },
  onShouldRefresh     : function(local, remote, dir) { ftpObserver.onShouldRefresh(local, remote, dir); },
  onDirNotFound       : function(buffer)             { },
  onChangeDir         : function(path, dontUpdateView, skipRecursion) { },
  onIsReadyChange     : function(state)              { },
  onTransferFail      : function(params, reason) {
    queueTree.addFailed(params, reason);
  },

  onSftpCache : function(buffer) {
    return 'n';
  }
};

function isReady() {
  for (var x = 0; x < gMaxCon; ++x) {
    if (!gConnections[x].isConnected) {
      continue;
    }

    if (!gConnections[x].isReady) {
      return false;
    }
  }

  return true;
}
