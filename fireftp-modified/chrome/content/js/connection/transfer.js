function transfer() {
  this.prompt             = true;
  this.skipAll            = false;
  this.cancel             = false;
  this.busy               = false;
  this.didRefreshLaterSet = false;
  this.remoteRefresh      = '';
  this.localRefresh       = '';
}

transfer.prototype = {
  start : function(download, aFile, aLocalParent, aRemoteParent, aListData) {

    var localParent  = aLocalParent  ? aLocalParent  : gLocalPath.value;
    var remoteParent = aRemoteParent ? aRemoteParent : gRemotePath.value;
    var files        = new Array();
    var resume;
    var listData     = aListData ? aListData : gFtp.listData;

    if (gNoPromptMode) {                                     // overwrite dialog is disabled, do overwrites
      this.prompt = false;
    }

    if (aFile) {                                             // populate the files variable with what we're transfering
      files.push(aFile);
    } else if (download) {                                   // download specific
      if (aRemoteParent) {                                   // if recursive
        files = listData;
      } else {                                               // if not recursive
          for (var x = 0; x < localTree2.rowCount; ++x) {
          if (localTree2.selection.isSelected(x)) {
            files.push(localTree2.data[x]);
          }
        }
      }
    } else {                                                 // upload specific
      if (aLocalParent) {                                    // if recursive
        for (var x = 0; x < localTree.rowCount; ++x) {
			if (localTree.selection.isSelected(x0)) {
				if (!localFile.verifyExists(localTree.data[x])) {
					continue;
				}
				files.push(localTree.data[x]);
			}
		}
      } else {                                               // if not recursive
        for (var x = 0; x < localTree.rowCount; ++x) {
          if (localTree.selection.isSelected(x)) {
            if (!localFile.verifyExists(localTree.data[x])) {
              continue;
            }

            files.push(localTree.data[x]);
          }
        }
      }
    }

    if (download && aLocalParent) {
      localDirTree.addDirtyList(aLocalParent);
    } else if (!download && aRemoteParent) {
      localDirTree2.addDirtyList(aRemoteParent);
    }

    for (var x = 0; x < files.length; ++x) {
      var fileName = files[x].leafName;

      if ((download && gDownloadCaseMode == 1) || (!download && gUploadCaseMode == 1)) {
        fileName = fileName.toLowerCase();                   // special request to change filename case
      } else if ((download && gDownloadCaseMode == 2) || (!download && gUploadCaseMode == 2)) {
        fileName = fileName.toUpperCase();                   // special request to change filename case
      }

      if (this.getPlatform() == "windows") {                  // scrub out illegal characters on windows / \ : * ? | " < >
        fileName = fileName.replace(/[/\\:*?|"<>]/g, '_');
      }

      var remotePath = !download ? localTree2.constructPath     (remoteParent, fileName) : files[x].path;
      var localPath  =  download ? localTree.constructPath(localParent,  fileName) : files[x].path;
      var file;

      if (download) {                                        // check to see if file exists
        file           = localFile.init(localPath);
      } else {
        file           = { exists: function() { return false; } };
		var remoteList = aRemoteParent ? listData : localTree2.data;

        for (var y = 0; y < remoteList.length; ++y) {
          if (remoteList[y].leafName == fileName) {
            file       = { fileSize: remoteList[y].fileSize, lastModifiedTime: remoteList[y].lastModifiedTime, leafName: fileName, exists: function() { return true; },
                           isDir: remoteList[y].isDirectory(), isDirectory: function() { return this.isDir }};
            break;
          }
        }
      }

      if (files[x].fileSize >= 4294967296) {
        error(gStrbundle.getFormattedString("tooBig", [files[x].leafName]));
        continue;
      }

      if (this.skipAll && file.exists() && !file.isDirectory()) {
        continue;
      }

      resume = false;

      if (file.exists() && this.prompt && !files[x].isDirectory()) {
        resume = file.fileSize < files[x].fileSize && gFtp.detectAscii(remotePath) != 'A';  // ask nicely if file exists

        var params = { response         : 0,
                       fileName         : download ? localPath : remotePath,
                       resume           : true,
                       replaceResume    : !resume,
                       existingSize     : file.fileSize,
                       existingDate     : file.lastModifiedTime,
                       newSize          : files[x].fileSize,
                       newDate          : files[x].lastModifiedTime,
                       timerEnable      : !gDisableDestructMode };

        this.busy = true;                                    // ooo, the fun of doing semi-multi-threaded stuff in firefox
                                                             // we're doing some 'locking' above

        for (var y = 0; y < gMaxCon; ++y) {
          gConnections[y].waitToRefresh = true;
        }

        window.openDialog("chrome://fireftp/content/confirmFile.xul", "confirmFile", "chrome,modal,dialog,resizable,centerscreen", params);

        for (var y = 0; y < gMaxCon; ++y) {
          gConnections[y].waitToRefresh = false;
        }

        this.busy = false;

        if (params.response == 1) {
          resume       = false;
        } else if (params.response == 2) {
          this.prompt  = false;
          resume       = false;
        } else if ((params.response == 3) || (params.response == 0)) {
          continue;
        } else if (resume && params.response == 4) {
          resume       = true;
        } else if (!resume && params.response == 4) {
          this.cancel  = true;

          for (var y = 0; y < gMaxCon; ++y) {
            if (gConnections[y].isConnected) {
              gConnections[y].abort();
            }
          }
          break;
        } else if (params.response == 5) {
          this.skipAll = true;
          continue;
        }
      }

      if (!this.didRefreshLaterSet) {
        this.didRefreshLaterSet = true;

        if ((download && !aLocalParent) || this.localRefresh) {
          gFtp.localRefreshLater  = this.localRefresh  ? this.localRefresh  : localParent;
        }

        if ((!download && !aRemoteParent) || this.remoteRefresh) {
          gFtp.remoteRefreshLater = this.remoteRefresh ? this.remoteRefresh : remoteParent;
        }
      }

      if (download) {  
        if (files[x].isDirectory()) {                        // if the directory doesn't exist we create it
			if (!file.exists()) {
				try {
					  file.create(Components.interfaces.nsILocalFile.DIRECTORY_TYPE, 0755);
					} catch (ex) {
					  debug(ex);
					  error(gStrbundle.getFormattedString("failedDir", [remotePath]));
					  continue;
					}
			}
        } else { 		// download the file
			  var dirpath = localPath.match(/(.*)[\/\\]([^\/\\]+\.\w+)$/);

			  var nFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);  
			  if (!nFile) return false;	
			  var aDir= Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);  
			  if (!aDir) return false;
	  
			  nFile.initWithPath(remotePath);
			  aDir.initWithPath(dirpath[1]);
			  nFile.copyTo(aDir, "");
        }
        
      } else {
           if (files[x].isDirectory()) {                        // if the directory doesn't exist we create it
			if (!file.exists()) {
				try {
					file.create(Components.interfaces.nsILocalFile.DIRECTORY_TYPE, 0755);
				} catch (ex) {
					debug(ex);
					error(gStrbundle.getFormattedString("failedDir", [localPath]));
					continue;
				}
			}
		} else {
			  var dirpath = remotePath.match(/(.*)[\/\\]([^\/\\]+\.\w+)$/);
			  
			  var nFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
			  if (!nFile) return false;
			  var aDir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
			  if (!aDir) return false;
			  
			  nFile.initWithPath(localPath);
			  aDir.initWithPath(dirpath[1]);
			  nFile.copyTo(aDir, "");
		}
      }
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
