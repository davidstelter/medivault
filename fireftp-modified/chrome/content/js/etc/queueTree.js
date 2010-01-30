var queueTree = {
  data                : new Array(),
  rowCount            : 0,
  failed              : new Array(),

  getParentIndex      : function(row)               { return -1; },
  getLevel            : function(row)               { return 0;  },
  getRowProperties    : function(row, props)        { },
  getColumnProperties : function(colid, col, props) { },
  isContainer         : function(row)               { return false; },
  isSeparator         : function(row)               { return false; },
  isSorted            : function(row)               { return false; },
  setTree             : function(treebox)           { this.treebox = treebox; },

  getCellText         : function(row, column)       {                                           // text for the files
    if (row >= 0 && row < this.data.length) {
      switch(column.id) {
        case "queuesource":
          return this.data[row].source;
        case "queuedest":
          return this.data[row].dest;
        case "queuebytes":
          return this.data[row].size;
        case "queueela":
          return this.data[row].ela;
        case "queuerem":
          return this.data[row].remain;
        case "queuerate":
          return this.data[row].rate;
        case "queuepercent":
          return "";
        case "queuetype":
          return this.data[row].type;
        case "queuestatus":
          return this.data[row].status;
        default:
          return " ";
      }
    }

    return "";
  },

  getCellValue : function(row, col) {
    if (row >= 0 && row < this.data.length && col.id == "queuepercent") {
      return this.data[row].percent;
    }

    return 0;
  }, 

  getImageSrc : function(row, col)  {
    return row >= 0 && row < this.data.length && col.id == "queuesource" && this.data[row].icon ? this.data[row].icon : "";
  },

  cycleHeader : function(col) { },

  getCellProperties : function(row, col, props)   {
    if (row >= 0 && row < this.data.length && this.data[row]) {
      if (col.id == "queuesource") {
        props.AppendElement(gAtomService.getAtom("nameCol"));
      }
    }
  },

  getProgressMode : function(row, col) {
    if (row >= 0 && row < this.data.length) {
      return this.data[row].mode == "determined" ? Components.interfaces.nsITreeView.PROGRESS_NORMAL : Components.interfaces.nsITreeView.PROGRESS_NONE;
    }

    return Components.interfaces.nsITreeView.PROGRESS_NONE;
  },  

  // ****************************************************** updateView ***************************************************

  updateView : function() {
		if ($('logQueueTabs').selectedIndex != 1) {
			return;
		}

    var selectedIds    = new Array();
    var queueTreeItems = new Array();
    var currentIndexId = this.selection.currentIndex >= 0 && this.selection.currentIndex < this.rowCount ? this.data[this.selection.currentIndex].id : -1;

    for (var x = 0; x < this.rowCount; ++x) {
      if (this.selection.isSelected(x)) {
        selectedIds.push(this.data[x].id);
      }
    }

    var found  = true;
    var count  = 0;
    var totalCount = 0;

    while (found) {
      found = false;

      for (var x = 0; x < gMaxCon; ++x) {
        if (count < gConnections[x].eventQueue.length) {
          found = true;

          if (gConnections[x].eventQueue[count].cmd == "transferEnd") {
            ++totalCount;
            if (totalCount > 100) {   // save on CPU, only show first 500
              break;
            }

            var info     = gConnections[x].eventQueue[count].callback;

            var leafName = info.remotePath.substring(info.remotePath.lastIndexOf('/') + 1);

            var obj = {
              id      : info.id,
              source  : info.type == "download" ? info.remotePath : info.localPath,
              dest    : info.type == "download" ? info.localPath  : info.remotePath,
              size    : commas(info.size),
              typeO   : info.transport,
              type    : (info.type == "download" ? gStrbundle.getString("download") : (info.type == "fxp" ? "FXP" : gStrbundle.getString("upload"))) + (info.ascii == "A" ? " (ASCII)": ''),
              icon    : "moz-icon://" + leafName + "?size=16",
              ela     : '',
              remain  : '',
              rate    : '',
              percent : '',
              status  : '',
              mode    : '',
              failed  : false
            };

            if ((gConnections[x].dataSocket && gConnections[x].dataSocket.id == info.id && info.type == "upload" && gConnections[x].dataSocket.progressEventSink.compressStream)
             || (info.transport == 'fxp'  && gFtp.eventQueue.length > 1 && gFtp.eventQueue[1].cmd == "transferEnd" && gFtp.eventQueue[1].callback.id == info.id)
             || (info.transport == 'sftp' && gFtp.eventQueue.length > 1 && gFtp.eventQueue[1].cmd == "transferEnd" && gFtp.eventQueue[1].callback.id == info.id)) {
              obj.status          = gStrbundle.getString("transferring")
                                 + (gConnections[x].security ? ", " + (gConnections[x].securityMode == "P" || gConnections[x].security == "sftp" ? gStrbundle.getString("dataEncrypted")
                                                                                                                                                 : gStrbundle.getString("dataNotEncrypted")) : "");
            } else if (gConnections[x].dataSocket && gConnections[x].dataSocket.id == info.id) {
              var bytesTotal;
              var bytesTransferred;
              var bytesPartial;
              var timeStart;

              if (info.type == "upload") {
                bytesTotal        = gConnections[x].dataSocket.progressEventSink.bytesTotal;
                bytesTransferred  = gConnections[x].dataSocket.progressEventSink.bytesUploaded;
                bytesPartial      = gConnections[x].dataSocket.progressEventSink.bytesPartial;
                timeStart         = gConnections[x].dataSocket.progressEventSink.timeStart;
              } else {
                bytesTotal        = gConnections[x].dataSocket.dataListener.bytesTotal;
                bytesTransferred  = gConnections[x].dataSocket.dataListener.bytesDownloaded;
                bytesPartial      = gConnections[x].dataSocket.dataListener.bytesPartial;
                timeStart         = gConnections[x].dataSocket.dataListener.timeStart;
              }

              if (bytesTotal) {
                var timeElapsed   = ((new Date()) - timeStart) / 1000;
                timeElapsed       = timeElapsed != 0 ? timeElapsed : 1;                         // no dividing by 0
                var averageRate   = ((bytesTransferred - bytesPartial) / 1024 / timeElapsed).toFixed(2);
                averageRate       = averageRate != 0 ? averageRate : "0.1";                     // no dividing by 0
                var timeRemaining = (bytesTotal - bytesTransferred) / 1024 * (1 / averageRate);
                averageRate       = averageRate.replace(/\./g, gStrbundle.getString("decimal")) + " " + gStrbundle.getString("kbsec");

                var hours         = parseInt( timeElapsed / 3600);
                var min           = parseInt((timeElapsed - hours * 3600) / 60);
                var sec           = parseInt( timeElapsed - hours * 3600 - min * 60);
                obj.ela           = zeros(hours) + ":" + zeros(min) + ":" + zeros(sec);

                hours             = parseInt( timeRemaining / 3600);
                min               = parseInt((timeRemaining - hours * 3600) / 60);
                sec               = parseInt( timeRemaining - hours * 3600 - min * 60);
                obj.remain        = zeros(hours) + ":" + zeros(min) + ":" + zeros(sec);

                obj.rate          = averageRate;
                var total         = bytesTotal != 0 ? bytesTotal : 1;                           // no dividing by 0
                var progress      = parseInt(bytesTransferred / total * 100) + "%";
                obj.mode          = "determined";
                obj.percent       = progress;
                obj.size          = progress + " - " + commas(bytesTransferred) + " / " + commas(bytesTotal);
                obj.status        = gStrbundle.getString("transferring")
                                  + (gConnections[x].security ? ", " + (gConnections[x].securityMode == "P" || gConnections[x].security == "sftp" ? gStrbundle.getString("dataEncrypted")
                                                                                                                                                  : gStrbundle.getString("dataNotEncrypted")) : "");
              } else if (gConnections[x].isConnected) {
                continue;
              }
            }

            queueTreeItems.push(obj);
          }
        }
      }

      ++count;
    }

    for (var x = 0; x < this.failed.length; ++x) {                                              // show failed transfers
      var info     = this.failed[x];

      var leafName = info.remotePath.substring(info.remotePath.lastIndexOf('/') + 1);

      var obj = {
        id      : info.id,
        source  : info.type == "download" ? info.remotePath : info.localPath,
        dest    : info.type == "download" ? info.localPath  : info.remotePath,
        size    : commas(info.size),
        typeO   : '',
        type    : (info.type == "download" ? gStrbundle.getString("download") : (info.type == "fxp" ? "FXP" : gStrbundle.getString("upload"))) + (info.ascii == "A" ? " (ASCII)": ''),
        icon    : "moz-icon://" + leafName + "?size=16",
        ela     : '',
        remain  : '',
        rate    : '',
        percent : '',
        status  : gStrbundle.getString("error") + ": " + (info.reason == "checksum" ? gStrbundle.getString("checksum") : info.reason),
        mode    : '',
        failed  : true
      };

      queueTreeItems.push(obj);
    }

    var oldCount  = this.rowCount;
    this.data     = queueTreeItems;                                                             // update queueTree
    this.rowCount = this.data.length;
    this.treebox.rowCountChanged(oldCount - 1, this.rowCount - oldCount);
    this.treebox.invalidate();

    this.selection.clearSelection();
    for (var x = 0; x < selectedIds.length; ++x) {                                              // reselect the rows that were selected
      for (var y = 0; y < this.rowCount; ++y) {
        if (selectedIds[x] == this.data[y].id && !this.selection.isSelected(y)) {
          this.selection.toggleSelect(y);
          break;
        }
      }
    }

    for (var x = 0; x < this.rowCount; ++x) {
      if (currentIndexId == this.data[x].id) {
        this.selection.currentIndex = x;
        break;
      }
    }
  },

  retry : function() {                                                                          // retry items from queue
    if (!gFtp.isConnected || this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      return;
    }

    var files = new Array();

    for (var x = 0; x < this.rowCount; ++x) {
      if (this.selection.isSelected(x) && this.data[x].failed) {
        for (var y = 0; y < this.failed.length; ++y) {
          if (this.data[x].id == this.failed[y].id) {
            files.push(this.failed[y]);
            this.failed.splice(y, 1);
            break;
          }
        }
      }
    }

    for (var x = 0; x < files.length; ++x) {
      if (files[x].type == "upload") {
        gFtp.upload(files[x].localPath, files[x].remotePath, false, files[x].size, -1);
      } else if (files[x].type == "download") {
        gFtp.download(files[x].remotePath, files[x].localPath, files[x].size, false, -1);
      }
    }
  },

  addFailed : function(params, reason) {
    params.reason = reason;
    this.failed.push(params);
  },

  cancel : function(zeFiles) {                                                                  // cancel items from queue
    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      return;
    }

    var files = new Array();

    if (zeFiles) {
      files = zeFiles;
    } else {
      for (var x = 0; x < this.rowCount; ++x) {
        if (this.selection.isSelected(x)) {
          files.push(this.data[x]);
        }
      }
    }

    for (var x = 0; x < files.length; ++x) {
      var connNo = files[x].id.split('-');
      connNo     = connNo[0] - 1;

      if (gConnections[connNo].dataSocket && gConnections[connNo].dataSocket.id == files[x].id) {
        gConnections[connNo].cancel();
      } else if (files[x].failed) {
        for (var y = 0; y < this.failed.length; ++y) {
          if (files[x].id == this.failed[y].id) {
            this.failed.splice(y, 1);
            break;
          }
        }
      } else {
        var begin = -1;
        var end   = -1;

        for (var y = 0; y < gConnections[connNo].eventQueue.length; ++y) {
          if (gConnections[connNo].eventQueue[y].cmd == "transferBegin" && gConnections[connNo].eventQueue[y].callback.id == files[x].id) {
            begin = y;
          } else if (gConnections[connNo].eventQueue[y].cmd == "transferEnd" && gConnections[connNo].eventQueue[y].callback.id == files[x].id) {
            end   = y;
            break;
          }
        }

        if (end != -1) {
          gConnections[connNo].eventQueue.splice(begin, end - begin + 1);
        }
      }
    }
  },

  // ************************************************* keyEvent *****************************************************

  keyPress : function(event) {
    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = 0;
    }

    if (testAccelKey(event) && (event.which == 65 || event.which == 97)) {
      event.preventDefault();                                                                   // accel-a: select all
      this.selection.selectAll();
    } else if (event.ctrlKey && event.keyCode == 32 && this.selection.count != 0) {             // ctrl-space, select or deselect
      this.selection.toggleSelect(this.selection.currentIndex);
    } else if (event.keyCode  == 46 && this.selection.count != 0) {                             // del
      this.cancel();
    } else if (event.keyCode  == 93) {                                                          // display context menu
      var x = {};    var y = {};    var width = {};    var height = {};
      this.treebox.getCoordsForCellItem(this.selection.currentIndex, this.treebox.columns["queuesource"], "text", x, y, width, height);
      this.createContextMenu();
      $('queuemenu').showPopup(gQueueTreeChildren, gQueueTreeChildren.boxObject.x + 75, gQueueTreeChildren.boxObject.y + y.value + 5, "context");
    }
  },

  createContextMenu : function() {
    var fxp   = false;
    var sftp  = false;
    var retry = false;

    for (var x = 0; x < this.rowCount; ++x) {
      if (!this.selection.isSelected(x)) {
        continue;
      }

      if (this.data[x].typeO == 'fxp') {
        fxp   = true;
        break;
      } else if (this.data[x].typeO == 'sftp') {
        sftp  = true;
        break;
      } else if (this.data[x].failed) {
        retry = true;
      }
    }

    $('queueRetry').setAttribute( "disabled", this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount || fxp || !retry);
    $('queueCancel').setAttribute("disabled", this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount || fxp || sftp);
  },

  // ************************************************* dropEvent *****************************************************

  canDrop : function(index, orient) {
    if (!gFtp.isConnected || !dragObserver.origin) {
      return false;
    }

    return true;
  },

  drop : function(index, orient) {
    if (dragObserver.origin == 'localtreechildren') {
      new transfer().start(false);
    } else if (dragObserver.origin == 'remotetreechildren') {
      new transfer().start(true);
    } else if (dragObserver.origin == 'external') {
      var transferObj           = new transfer();
      transferObj.remoteRefresh = gRemotePath.value;

      for (var x = 0; x < dragObserver.externalFiles.length; ++x) {
        var droppedFile    = dragObserver.externalFiles[x];
        var fileParent     = droppedFile.parent ? droppedFile.parent.path : "";

        transferObj.start(false, droppedFile, fileParent, gRemotePath.value);

        if (transferObj.cancel) {
          break;
        }
      }
    }
  }
};
