var remoteTree = {
  data          : new Array(),
  displayData   : new Array(),
  rowCount      : 0,
  remoteSize    : 0,
  searchMode    : 0,
  isEditing     : false,
  editType      : "",
  editParent    : null,
  extraCallback : null,
  errorCallback : null,
  rememberSort  : null,
  isLoading     : true,

  getParentIndex      : function(row)               { return -1; },
  getLevel            : function(row)               { return 0;  },
  getRowProperties    : function(row, props)        { },
  getColumnProperties : function(colid, col, props) { },
  isContainer         : function(row)               { return false; },
  isSeparator         : function(row)               { return false; },
  isSorted            : function(row)               { return false; },
  setTree             : function(treebox)           { this.treebox = treebox; },

  getCellText         : function(row, column)       {                                          // text for the files
    if (row >= 0 && row < this.data.length) {
      switch (column.id) {
        case "remotename":
          return this.searchMode == 2 ? this.displayData[row].path : this.displayData[row].leafName;
        case "remotesize":
          return this.displayData[row].fileSize;
        case "remotedate":
          return this.displayData[row].date;
        case "remotetype":
          return this.displayData[row].extension;
        case "remoteattr":
          return this.displayData[row].attr;
        default:
          return " ";
      }
    }

    return "";
  },

  getImageSrc : function(row, col) {
    return row >= 0 && row < this.data.length && col.id == "remotename" && this.displayData[row].icon ? this.displayData[row].icon : "";
  },

  cycleHeader : function(col) {
    var sortDirection = col.element.getAttribute("sortDirection") == "descending"
                     || col.element.getAttribute("sortDirection") == "natural"  ? "ascending" : "descending";
    $('remotename').setAttribute("sortDirection", "natural");
    $('remotesize').setAttribute("sortDirection", "natural");
    $('remotedate').setAttribute("sortDirection", "natural");
    $('remotetype').setAttribute("sortDirection", "natural");
    $('remoteattr').setAttribute("sortDirection", "natural");
    col.element.setAttribute(    "sortDirection", sortDirection);
    this.sort();
  },

  getCellProperties : function(row, col, props)   {
    if (row >= 0 && row < this.data.length) {
      if (col.id == "remotename") {
        if (this.data[row].isDirectory()) {
          props.AppendElement(gAtomService.getAtom("isFolder"));
        } else if (this.data[row].isSymlink()) {
          props.AppendElement(gAtomService.getAtom("isLink"));
        }

        props.AppendElement(gAtomService.getAtom("nameCol"));
      }

      if (dragObserver.overName && this.data[row].isDirectory()) {
        props.AppendElement(gAtomService.getAtom("overName"));
      }

      /*if (!gFtp.isConnected) {
        props.AppendElement(gAtomService.getAtom("disconnected"));
      }*/

      if (this.data[row].isHidden) {
        props.AppendElement(gAtomService.getAtom("hidden"));
      }

      if (this.data[row].isCut) {
        props.AppendElement(gAtomService.getAtom("cut"));
      }
    }
  },

  // ****************************************************** updateView ***************************************************

  updateView : function(skipCache) {
    this.isLoading = true;
    gFtp.list(gRemotePath.value, "remoteTree.updateView2()", skipCache);
  },

  updateView2 : function(files) {
    var remoteTreeItems;
    var firstSearch;

    this.isLoading = false;

    if (!files) {
      this.searchMode = 0;
      gRemoteTreeChildren.removeAttribute('search')

      remoteTreeItems    = gFtp.listData;
      this.remoteSize    = 0;                                                                     // get directory size

      for (var x = 0; x < remoteTreeItems.length; ++x) {
        this.remoteSize += remoteTreeItems[x].fileSize;
      }

      this.remoteSize    = parseSize(this.remoteSize);
      this.data          = remoteTreeItems;
    } else {
      if (this.remoteSize != -1) {
        this.data        = new Array();
        this.displayData = new Array();
        this.treebox.rowCountChanged(0, -this.rowCount);

        this.rememberSort = { cols : ["remotename", "remotesize", "remotedate", "remotetype", "remoteattr"],
                              vals : [$('remotename').getAttribute("sortDirection"),
                                      $('remotesize').getAttribute("sortDirection"),
                                      $('remotedate').getAttribute("sortDirection"),
                                      $('remotetype').getAttribute("sortDirection"),
                                      $('remoteattr').getAttribute("sortDirection")] };
      }

      files.sort(compareName);

      for (var x = 0; x < files.length; ++x) {
        this.data.push(files[x]);
      }

      this.remoteSize  = -1;
      this.searchMode  = this.searchMode ? this.searchMode : (gSearchRecursive ? 2 : 1);
      gRemoteTreeChildren.setAttribute('search', true);
    }

    this.sort(files);                                                                           // update remoteTree

    var index = remoteDirTree.indexOfPath(gRemotePath.value);                                   // select directory in remoteDirTree
    remoteDirTree.selection.select(index);
    remoteDirTree.treebox.ensureRowIsVisible(index);

    if (this.data.length && !files) {
      this.selection.select(0);                                                                 // select first element in remoteTree
    }

    this.mouseOver(null);

    if (files) {
      return;
    }

    var anyFolders = false;                                                                     // see if the folder has any subfolders
    for (var x = 0; x < this.data.length; ++x) {
      if (this.data[x].isDirectory()) {
        anyFolders = true;
        break;
      }
    }

    if (!anyFolders) {                                                                          // and if there are no subfolders then update our tree
      if (remoteDirTree.data[index].open) {                                                     // if remoteDirTree is open
        remoteDirTree.toggleOpenState(index);
      }

      remoteDirTree.data[index].empty     = true;
      remoteDirTree.data[index].open      = false;
      remoteDirTree.data[index].children  = null;

      for (var x = 0; x < remoteDirTree.dirtyList.length; ++x) {
        if (remoteDirTree.dirtyList[x] == gRemotePath.value) {
          remoteDirTree.dirtyList.splice(x, 1);
          break;
        }
      }
    } else if (anyFolders && remoteDirTree.data[index].empty) {
      remoteDirTree.data[index].empty     = false;
    }

    remoteDirTree.treebox.invalidateRow(index);

    if (this.extraCallback) {
      var tempCallback   = this.extraCallback;
      var func = function() { tempCallback(); };
      this.extraCallback = null;
      setTimeout(func, 0);
    }
  },

  sort : function(files) {
    if (!files) {
      if (this.rememberSort) {
        for (var x = 0; x < this.rememberSort.cols.length; ++x) {
          $(this.rememberSort.cols[x]).setAttribute("sortDirection", this.rememberSort.vals[x]);
        }

        this.rememberSort = null;
      }

      this.sortHelper($('remotename'), this.searchMode == 2 ? directorySort2 : compareName);
      this.sortHelper($('remotesize'), compareSize);
      this.sortHelper($('remotedate'), compareDate);
      this.sortHelper($('remotetype'), compareType);
      this.sortHelper($('remoteattr'), compareRemoteAttr);

      this.displayData = new Array();
    } else {
      $('remotename').setAttribute("sortDirection", "natural");
      $('remotesize').setAttribute("sortDirection", "natural");
      $('remotedate').setAttribute("sortDirection", "natural");
      $('remotetype').setAttribute("sortDirection", "natural");
      $('remoteattr').setAttribute("sortDirection", "natural");
    }

    var start = files ? this.data.length - files.length : 0;

    for (var row = start; row < this.data.length; ++row) {
      this.displayData.push({ leafName : this.data[row].leafName,
                              fileSize : this.getFormattedFileSize(row),
                              date     : this.data[row].date,
                              extension: this.data[row].isDirectory() ? "" : this.getExtension(this.data[row].leafName),
                              attr     : this.data[row].permissions,
                              icon     : this.getFileIcon(row),
                              path     : this.data[row].path });
    }

    if (files) {
      this.rowCount = this.data.length;
      this.treebox.rowCountChanged(start, files.length);
    } else {
      this.treebox.rowCountChanged(0, -this.rowCount);
      this.rowCount = this.data.length;
      this.treebox.rowCountChanged(0, this.rowCount);
    }
  },

  sortHelper : function(el, sortFunc) {
    if (el.getAttribute("sortDirection") && el.getAttribute("sortDirection") != "natural") {
      this.data.sort(sortFunc);

      if (!gPrefs.getBoolPref("remotesortfix")) {   // blah, fix dumb mistake that changed descending into ascending
        el.setAttribute("sortDirection", "ascending");
        gPrefs.setBoolPref("remotesortfix", true);
      }

      if (el.getAttribute("sortDirection") == "descending") {
        this.data.reverse();
      }
    }
  },

  getFormattedFileSize : function(row) {
    if (!this.data[row].fileSize) {
      return gBytesMode ? "0  " : gStrbundle.getFormattedString("kilobyte", ["0"]) + "  ";
    }

    if (gBytesMode) {
      return commas(this.data[row].fileSize) + "  ";
    }

    return gStrbundle.getFormattedString("kilobyte", [commas(Math.ceil(this.data[row].fileSize / 1024))]) + "  ";
  },

  getExtension : function(leafName) {
    return leafName.lastIndexOf(".") != -1 ? leafName.substring(leafName.lastIndexOf(".") + 1, leafName.length).toLowerCase() : "";
  },

  getFileIcon : function(row) {
    return this.data[row].isDirectory() || this.data[row].isSymlink() ? "" :  "moz-icon://" + this.data[row].leafName + "?size=16";
  },

  // ****************************************************** refresh ***************************************************

  refresh : function() {
    /*if (!gFtp.isConnected) {
      return;
    }*/

    if (remoteDirTree.selection.currentIndex == -1) {
      return;
    }

    if (remoteDirTree.data[remoteDirTree.selection.currentIndex].open) {                        // if remoteDirTree is open
      gFtp.removeCacheEntry(remoteDirTree.data[remoteDirTree.selection.currentIndex].path);     // clear out cache entry
      gFtp.list(remoteDirTree.data[remoteDirTree.selection.currentIndex].path, "remoteTree.refreshCallback(" + remoteDirTree.selection.currentIndex + ")", false);      // get data for this directory
    } else {
      remoteDirTree.data[remoteDirTree.selection.currentIndex].empty    = false;                // not empty anymore
      remoteDirTree.data[remoteDirTree.selection.currentIndex].children = null;                 // reset its children
      remoteDirTree.treebox.invalidateRow(remoteDirTree.selection.currentIndex);
      this.updateView(true);
    }
  },

  refreshCallback : function(index) {
    remoteDirTree.toggleOpenState(remoteDirTree.selection.currentIndex);                      // close it up
    remoteDirTree.data[remoteDirTree.selection.currentIndex].children = null;                 // reset its children
    remoteDirTree.updateViewAfter = true;                                                     // refresh remoteTree afterwards
    remoteDirTree.toggleOpenState2(index);                                                    // and open it up again
  },

  // ************************************************* file functions ***************************************************

  launch : function() {
    /*if (this.selection.count == 0 || !gFtp.isConnected || !isReady()) {
      return;
    }*/

    try {
      var count = 0;

      for (var x = 0; x < remoteTree.rowCount; ++x) {
        if (remoteTree.selection.isSelected(x)) {
          ++count;

          let tmpFile = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties).get("TmpD", Components.interfaces.nsILocalFile);
          tmpFile.append(count + '-' + remoteTree.data[x].leafName);
          while (tmpFile.exists()) {
            ++count;
            tmpFile.leafName = count + '-' + remoteTree.data[x].leafName;
          }

          count = 0;

          let remoteFile = remoteTree.data[x];

          var func = function() {
            var subFunc = function() { launchProgram(null, null, tmpFile, remoteFile); };
            setTimeout(subFunc, 0);                                                                     // let the queue finish up
          };

          gFtp.download(remoteFile.path, tmpFile.path, remoteFile.fileSize, false, 0, false, func);
        }
      }
    } catch (ex) {
      debug(ex);
    }

  },

  openContainingFolder : function() {
    if (!isReady() || this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      return;
    }

    remoteDirTree.changeDir(this.data[this.selection.currentIndex].parent.path);
  },

  viewOnTheWeb : function() {
    /*if (!gFtp.isConnected || this.selection.count == 0) {
      return;
    }*/

    if (!gWebHost) {
      doAlert(gStrbundle.getString("fillInWebHost"));
      return;
    }

    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = this.rowCount - 1;
    }

    for (var x = 0; x < remoteTree.rowCount; ++x) {
      if (remoteTree.selection.isSelected(x)) {
        var path = this.data[x].path;

        if (gPrefix && path.indexOf(gPrefix) == 0) {
          path = path.substring(gPrefix.length);
        }

        runInFirefox(gWebHost + escape(gFtp.fromUTF8.ConvertFromUnicode(path) + gFtp.fromUTF8.Finish()));
      }
    }
  },

  copyUrl : function(http, login) {
    /*if (!gFtp.isConnected || this.selection.count == 0) {
      return;
    }*/

    if (http && !gWebHost) {
      doAlert(gStrbundle.getString("fillInWebHost"));
      return;
    }

    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = this.rowCount - 1;
    }

    var paths = "";

    for (var x = 0; x < remoteTree.rowCount; ++x) {
      if (remoteTree.selection.isSelected(x)) {
        var path = this.data[x].path;

        if (http && gPrefix && path.indexOf(gPrefix) == 0) {
          path = path.substring(gPrefix.length);
        }

        path = http ? gWebHost + escape(gFtp.fromUTF8.ConvertFromUnicode(path) + gFtp.fromUTF8.Finish())
                    : 'ftp://' + (login ? encodeURIComponent(gFtp.login) + ':' + gFtp.password + '@' : '')
                               + gFtp.host + (gFtp.port == 21 ? '' : ':' + gFtp.port) + path;

        paths += (paths ? '\n' : '') + path;
      }
    }


    var clipboard = Components.classes["@mozilla.org/widget/clipboardhelper;1"].createInstance(Components.interfaces.nsIClipboardHelper);
    clipboard.copyString(paths);
  },

  create : function(isDir) {
    /*if (!gFtp.isConnected || !isReady() || this.searchMode == 2) {
      return;
    }*/

    this.data.push({        leafName    : "",
                            fileSize    : "",
                            date        : "",
                            extension   : "",
                            attr        : "",
                            path        : "",
                            isDir       : isDir,
                            isDirectory : function() { return true  },
                            isSymlink   : function() { return false },
                            isHidden    : false });
    this.displayData.push({ leafName    : "",
                            fileSize    : "",
                            date        : "",
                            extension   : "",
                            attr        : "",
                            icon        : isDir ? "" : "moz-icon://file?size=16",
                            path        : "" });
    ++this.rowCount;
    this.treebox.rowCountChanged(this.rowCount - 1, 1);
    this.treebox.ensureRowIsVisible(this.rowCount - 1);

    this.editType   = "create";
    this.editParent = gRemotePath.value;
    setTimeout("gRemoteTree.startEditing(remoteTree.rowCount - 1, gRemoteTree.columns['remotename'])", 0);
  },

  remove : function() {
    /*if (!gFtp.isConnected || !isReady() || this.selection.count == 0 || this.rowCount == 0) {
      return;
    }*/

    if (gRemoteTree.view.selection.count > 1) {                                                 // deleting multiple
      if (!window.confirm(gStrbundle.getFormattedString("confirmDelete2", [gRemoteTree.view.selection.count]))) {
        return;
      }
    } else if (this.data[gRemoteTree.view.selection.currentIndex].isDirectory()) {              // deleting a directory
      if (!window.confirm(gStrbundle.getFormattedString("confirmDelete3", [this.data[gRemoteTree.view.selection.currentIndex].leafName]))) {
        return;
      }
    } else {                                                                                    // deleting a file
      if (!window.confirm(gStrbundle.getFormattedString("confirmDelete", [this.data[gRemoteTree.view.selection.currentIndex].leafName]))) {
        return;
      }
    }

    var last  = true;

    gFtp.beginCmdBatch();

    for (var x = 0; x < this.rowCount; ++x) {
      if (this.selection.isSelected(x)) {
        if (last) {
          gFtp.changeWorkingDirectory(gRemotePath.value);
        }

        gFtp.remove(this.data[x].isDirectory(),
                    this.data[x].path,
                    (last && gRefreshMode) ? "remoteTree.refresh()" : "");
        last = false;
      }
    }

    gFtp.endCmdBatch();
  },

  rename : function() {
    /*if (!gFtp.isConnected || !isReady() || this.selection.count == 0 || this.rowCount == 0) {
      return;
    }*/

    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = this.rowCount - 1;
    }

    this.displayData[this.selection.currentIndex].origLeafName = this.data[this.selection.currentIndex].leafName;
    this.displayData[this.selection.currentIndex].origPath     = this.data[this.selection.currentIndex].path;

    if (this.searchMode == 2) {
      this.displayData[this.selection.currentIndex].path = this.displayData[this.selection.currentIndex].leafName;
      this.treebox.invalidateRow(this.selection.currentIndex);
    }

    this.editType   = "rename";
    this.editParent = gRemotePath.value;
    gRemoteTree.startEditing(this.selection.currentIndex, gRemoteTree.columns["remotename"]);
  },

  isEditable : function(row, col) {
    var canEdit = row >= 0 && row < this.data.length && col.id == "remotename";
    this.isEditing = canEdit;
    return canEdit;
  },

  setCellText : function(row, col, val) {
    if (!this.isEditing || this.editParent != gRemotePath.value) {                              // for some reason, this is called twice - so we prevent this
      return;
    }

    this.isEditing = false;
    if (this.editType == "rename") {
      if (this.data[row].leafName == val) {
        // do nothing
      } else {
        var path       = this.data[row].path;
        var newName    = val;

        if (!newName) {
          return;
        }

        for (var x = 0; x < this.rowCount; ++x) {
          if (this.data[x].leafName == newName) {
            error(gStrbundle.getString("renameFail"));
            this.displayData[row].leafName = val;
            this.treebox.invalidateRow(row);
            setTimeout("gRemoteTree.startEditing(" + row + ", gRemoteTree.columns['remotename'])", 0);
            return;
          }
        }

        if (path.charAt(path.length - 1) == '/') {
          path = path.substring(path.length - 1);
        }

        newName = path.substring(0, path.lastIndexOf('/')) + '/' + newName;

        var rowDiff        = this.treebox.getLastVisibleRow() - row;
        var self           = this;
        var renameCallback = function() {
          for (var x = 0; x < self.rowCount; ++x) {
            if (self.data[x].leafName == val) {
              self.selection.select(x);
              self.treebox.ensureRowIsVisible(rowDiff + x - 1 < self.rowCount ? rowDiff + x - 1 : self.rowCount - 1);
              break;
            }
          }
        };
        remoteTree.extraCallback = renameCallback;

        var errorCallback = function() {
          setTimeout("gRemoteTree.startEditing(" + row + ", gRemoteTree.columns['remotename'])", 0);
        };
        remoteTree.errorCallback = errorCallback;

        this.displayData[row].leafName = val;
        this.treebox.invalidateRow(row);
        gFtp.rename(path, newName, gRefreshMode ? "remoteTree.refresh()" : "", this.data[row].isDirectory());
      }
    } else if (this.editType == "create") {
      if (val) {
        var self           = this;
        var createCallback = function() {
          for (var x = 0; x < self.rowCount; ++x) {
            if (self.data[x].leafName == val) {
              self.selection.select(x);
              self.treebox.ensureRowIsVisible(x);
              break;
            }
          }
        };
        remoteTree.extraCallback = createCallback;

        var errorCallback = function() {
          self.data[row].leafName        = val;
          self.displayData[row].leafName = val;
          self.treebox.invalidateRow(row);
          setTimeout("gRemoteTree.startEditing(remoteTree.rowCount - 1, gRemoteTree.columns['remotename'])", 0);
        };
        remoteTree.errorCallback = errorCallback;

        this.displayData[row].leafName = val;
        this.treebox.invalidateRow(row);
        if (this.data[row].isDir) {
          gFtp.makeDirectory(gFtp.constructPath(gRemotePath.value, val), gRefreshMode ? "remoteTree.refresh()" : "");
        } else {
          gFtp.makeBlankFile(gFtp.constructPath(gRemotePath.value, val), gRefreshMode ? "remoteTree.refresh()" : "");
        }
      } else {
        --this.rowCount;
        this.data.splice(this.rowCount, 1);
        this.displayData.splice(this.rowCount, 1);
        this.treebox.rowCountChanged(this.rowCount, -1);
      }
    }
  },

  showProperties : function(recursive) {
    /*if (!gFtp.isConnected || !isReady() || this.selection.count == 0 || this.rowCount == 0) {
      return;
    }*/

    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = this.rowCount - 1;
    }

    this.recursiveFolderData = { type: "remote", nFolders: 0, nFiles: 0, nSize: 0, files: new Array() };

    if (this.selection.count > 1) {                                                             // multiple files
      var last = true;

      for (var x = 0; x < this.rowCount; ++x) {
        if (this.selection.isSelected(x)) {
          if (this.data[x].isDirectory()) {
            ++this.recursiveFolderData.nFolders;

            if (recursive) {
              var remotePath = this.data[x].path;

              if (last) {
                gFtp.beginCmdBatch();
              }

              gFtp.list(remotePath, "remoteTree.getRecursiveFolderData('" + remotePath.replace(/'/g, "\\'") + "'," + last + ")", true, true);
              last = false;
            }

          } else {
            ++this.recursiveFolderData.nFiles;
          }

          this.recursiveFolderData.files.push(this.data[x]);
          this.recursiveFolderData.nSize += this.data[x].fileSize;
        }
      }

      if (last) {
        this.showMultipleProperties();
      } else {
        gFtp.endCmdBatch();
      }

      return;
    }

    var index = this.selection.currentIndex;
    var path  = this.data[index].path;

    if (this.data[index].isDirectory() && recursive) {                                          // directory
      gFtp.list(path, "remoteTree.getRecursiveFolderData('" + path.replace(/'/g, "\\'") + "', true, true)", true);
      this.recursiveFolderData.nSize += this.data[index].fileSize;
      return;
    }

    var params = { path            : path,
                   leafName        : this.data[index].leafName,
                   fileSize        : this.data[index].fileSize,
                   date            : this.data[index].date,
                   origPermissions : this.data[index].permissions,
                   writable        : 'disabled',
                   hidden          : 'disabled',
                   isDirectory     : this.data[index].isDirectory(),
                   user            : this.data[index].user,
                   group           : this.data[index].group,
                   permissions     : "",
                   webHost         : gWebHost,
                   prefix          : gPrefix,
                   isSymlink       : this.data[index].isSymlink(),
                   symlink         : this.data[index].symlink,
                   featXMD5        : gFtp.featXMD5,
                   featXSHA1       : gFtp.featXSHA1,
                   gFtp            : gFtp };

    window.openDialog("chrome://fireftp/content/properties.xul", "properties", "chrome,modal,dialog,resizable,centerscreen", params);

    if (params.permissions && gFtp.isConnected && isReady()) {                                  // permissions were changed; CHMOD!
      var rowDiff        = this.treebox.getLastVisibleRow() - index;
      var name           = this.data[index].leafName;
      var self           = this;
      var propsCallback  = function() {
        for (var x = 0; x < self.rowCount; ++x) {
          if (self.data[x].leafName == name) {
            self.selection.select(x);
            self.treebox.ensureRowIsVisible(rowDiff + x - 1 < self.rowCount ? rowDiff + x - 1 : self.rowCount - 1);
            break;
          }
        }
      };
      remoteTree.extraCallback = propsCallback;

      gFtp.changePermissions(params.permissions, path, gRefreshMode ? "remoteTree.refresh()" : "");
    }
  },

  recursiveFolderData    : new Object(),
  getRecursiveFolderData : function(parent, last, showDir) {
    var files = gFtp.listData;

    for (var x = 0; x < files.length; ++x) {
      var remotePath = gFtp.constructPath(parent, files[x].leafName);

      if (files[x].isDirectory()) {
        ++this.recursiveFolderData.nFolders;
        gFtp.list(remotePath, "remoteTree.getRecursiveFolderData('" + remotePath.replace(/'/g, "\\'") + "'," + last + "," + showDir + ")", true, true);
        last = false;
      } else {
        ++this.recursiveFolderData.nFiles;
      }

      this.recursiveFolderData.files.push(files[x]);
      this.recursiveFolderData.nSize += files[x].fileSize;
    }

    if (last) {
      this.showMultipleProperties(showDir);
    }
  },

  showMultipleProperties : function(dir) {
    var params;
    var path;

    if (dir) {
      var index = this.selection.currentIndex;
      path      = this.data[index].path;

      params = { path                : path,
                 leafName            : this.data[index].leafName,
                 fileSize            : 0,
                 date                : this.data[index].date,
                 origPermissions     : this.data[index].permissions,
                 writable            : 'disabled',
                 hidden              : 'disabled',
                 isDirectory         : this.data[index].isDirectory(),
                 user                : this.data[index].user,
                 group               : this.data[index].group,
                 permissions         : "",
                 isSymlink           : this.data[index].isSymlink(),
                 symlink             : this.data[index].symlink,
                 multipleFiles       : false,
                 recursiveFolderData : this.recursiveFolderData,
                 applyTo             : { type: "remote", thisFile: true, folders: false, files: false } };
    } else {
      params = { multipleFiles       : true,
                 recursiveFolderData : this.recursiveFolderData,
                 permissions         : "",
                 applyTo             : { folders: false, files: false } };
    }

    window.openDialog("chrome://fireftp/content/properties.xul", "properties", "chrome,modal,dialog,resizable,centerscreen", params);

    if (params.permissions && gFtp.isConnected) {                                               // permissions were changed; CHMOD!
      var last = true;

      gFtp.beginCmdBatch();

      if (dir && params.applyTo.thisFile) {
        gFtp.changePermissions(params.permissions, path, gRefreshMode ? "remoteTree.refresh()" : "");
        last = false;
      }

      for (var x = 0; x < this.recursiveFolderData.files.length; ++x) {
        if ((this.recursiveFolderData.files[x].isDirectory() && params.applyTo.folders)
        || (!this.recursiveFolderData.files[x].isDirectory() && params.applyTo.files)) {
          gFtp.changePermissions(params.permissions, this.recursiveFolderData.files[x].path, (last && gRefreshMode) ? "remoteTree.refresh()" : "");
          last = false;
        }
      }

      gFtp.endCmdBatch();
    }
  },

  // ************************************************* mouseEvent *****************************************************

  dblClick : function(event) {
    /*if (!gFtp.isConnected || event.button != 0 || event.originalTarget.localName != "treechildren" || this.selection.count == 0) {
      return;
    }*/

    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = this.rowCount - 1;
    }

    if (this.data[this.selection.currentIndex].isDirectory()) {                                 // if it's a directory
      if (!isReady()) {
        return;
      }
                                                                                                // navigate to it
      remoteDirTree.changeDir(this.data[this.selection.currentIndex].path);
    } else if (this.data[this.selection.currentIndex].isSymlink()) {                            // if it's a symbolic link
      if (gFtp.isListing()) {
        return;
      }

      var linkedPath = this.data[this.selection.currentIndex].path;
      var linkedFile = this.data[this.selection.currentIndex].symlink;
      var parentPath = gRemotePath.value;

      while (linkedFile.indexOf("./") == 0 || linkedFile.indexOf("../") == 0) {
        if (linkedFile.indexOf("./") == 0) {
          linkedFile = linkedFile.substring(2);
        } else {
          linkedFile = linkedFile.substring(3);
          parentPath = parentPath.substring(0, parentPath.lastIndexOf('/') ? parentPath.lastIndexOf('/') : 1);
        }
      }

      if (linkedFile.indexOf("/") != 0) {
        linkedFile = gFtp.constructPath(parentPath, linkedFile);
      }

      parentPath = linkedFile.substring(0, linkedFile.lastIndexOf('/') ? linkedFile.lastIndexOf('/') : 1);

      var self    = this;
      var cwdFunc = function(success) {
        if (success) {
          return;
        }

        var cwd2Func = function(success2) {
          var listFunc = function() {
            for (var x = 0; x < gFtp.listData.length; ++x) {
              if (gFtp.listData[x].path == linkedFile) {
                new transfer().start(true, gFtp.listData[x], '', parentPath);
                return;
              }
            }

            gFtp.changeWorkingDirectory(linkedPath);
          };

          if (success2) {
            gFtp.list(parentPath, listFunc);
          } else {
            gFtp.changeWorkingDirectory(linkedPath);
          }
        };

        if (gFtp.currentWorkingDir != parentPath) {
          gFtp.changeWorkingDirectory(parentPath, cwd2Func);
        } else {
          cwd2Func(true);
        }
      };

      if (gFtp.currentWorkingDir != linkedFile) {
        gFtp.changeWorkingDirectory(linkedFile, cwdFunc);
      } else {
        remoteDirTree.changeDir(gFtp.currentWorkingDir);
      }
    } else {
      if (gOpenMode) {
        this.launch();
      } else {
        new transfer().start(true);                                                             // else download the file
      }
    }
  },

  click : function(event) {
    if (gFtp.isConnected && event.button == 1 && !$('remotePasteContext').disabled) {           // middle-click paste
      this.paste();
    }
  },

  createContextMenu : function() {
    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = this.rowCount - 1;
    }

    for (var x = $('remoteOpenWithMenu').childNodes.length - 1; x >= 0; --x) {                  // clear out the menus
      $('remoteOpenWithMenu').removeChild($('remoteOpenWithMenu').childNodes.item(x));
    }

    for (var x = $('remoteFXPMenu').childNodes.length - 1; x >= 0; --x) {
      $('remoteFXPMenu').removeChild($('remoteFXPMenu').childNodes.item(x));
    }

    $('remoteOpenCont').collapsed    =               this.searchMode != 2;
    $('remoteOpenContSep').collapsed =               this.searchMode != 2;
    /*$('remoteCutContext').setAttribute("disabled",   this.searchMode == 2 || !gFtp.isConnected);
    $('remotePasteContext').setAttribute("disabled", this.searchMode == 2 || !gFtp.isConnected || !this.pasteFiles.length);
    $('remoteCreateDir').setAttribute("disabled",    this.searchMode == 2 || !gFtp.isConnected);
    $('remoteCreateFile').setAttribute("disabled",   this.searchMode == 2 || !gFtp.isConnected);*/

    if (this.selection.currentIndex == -1) {
      return;
    }

    var hasDir = false;
    for (var x = 0; x < this.rowCount; ++x) {
      if (this.selection.isSelected(x)) {
        if (this.data[x].isDirectory()) {
          hasDir = true;
          break;
        }
      }
    }

    /*$('remoteRecursiveProperties').setAttribute("disabled", !hasDir || !gFtp.isConnected);*/

    var extension = this.getExtension(this.data[this.selection.currentIndex].leafName);
    var item;
    var found     = false;

    var self = this;
    var contextMenuHelper = function(x, y) {
      found = true;
      var program = localFile.init(gPrograms[x].programs[y].executable);

      if (!program) {
        return;
      }

      var fileURI = gIos.newFileURI(program);
      item        = document.createElement("menuitem");
      item.setAttribute("class",     "menuitem-iconic");
      item.setAttribute("image",     "moz-icon://" + fileURI.spec + "?size=16");
      item.setAttribute("label",     gPrograms[x].programs[y].name);
      item.setAttribute("oncommand", "remoteLaunchProgram(" + x + ", " + y + ", " + self.selection.currentIndex + ")");
      $('remoteOpenWithMenu').appendChild(item);
    };

    for (var x = 0; x < gPrograms.length; ++x) {
      if (gPrograms[x].extension.toLowerCase() == extension.toLowerCase()) {
        for (var y = 0; y < gPrograms[x].programs.length; ++y) {
          contextMenuHelper(x, y);
        }

        break;
      }
    }

    for (var x = 0; x < gPrograms.length; ++x) {
      if (gPrograms[x].extension == "*.*") {
        for (var y = 0; y < gPrograms[x].programs.length; ++y) {
          contextMenuHelper(x, y);
        }

        break;
      }
    }

    if (found) {
      item = document.createElement("menuseparator");
      $('remoteOpenWithMenu').appendChild(item);
    }

    item = document.createElement("menuitem");
    item.setAttribute("label", gStrbundle.getString("chooseProgram"));
    item.setAttribute("oncommand", "chooseProgram(true)");
    $('remoteOpenWithMenu').appendChild(item);

    for (var x = 0; x < gSiteManager.length; ++x) {
      if (gSiteManager[x].account != gAccount) {
        item = document.createElement("menuitem");
        item.setAttribute("class",     "menuitem-iconic");
        item.setAttribute("image",     "chrome://fireftp/skin/icons/logo.png");
        item.setAttribute("label",     gSiteManager[x].account);
        item.setAttribute("oncommand", "fxp('" + gSiteManager[x].account + "')");
        $('remoteFXPMenu').appendChild(item);
      }
    }
  },

  mouseOver : function(event) {                                                                 // display remote folder info
    if (this.rowCount) {
      $('statustxt').label = gStrbundle.getString("remoteListing") + " " + gStrbundle.getFormattedString("objects", [this.rowCount])
                           + (this.remoteSize < 0 ? "" : ", " + commas(this.remoteSize));
    } else {
      $('statustxt').label = gStrbundle.getString("remoteListingNoObjects");
    }
  },

  // ************************************************* keyEvent *****************************************************

  keyPress : function(event) {
    /*if (!gFtp.isConnected) {
      return;
    }*/

    if (gRemoteTree.editingRow != -1) {
      if (event.keyCode == 27) {
        if (this.editType == "create") {
          this.setCellText(-1, "", "");
        } else {
          this.displayData[gRemoteTree.editingRow].leafName = this.displayData[gRemoteTree.editingRow].origLeafName;
          this.displayData[gRemoteTree.editingRow].path     = this.displayData[gRemoteTree.editingRow].origPath;
          this.treebox.invalidateRow(gRemoteTree.editingRow);
        }
      }

      return;
    }

    if (this.selection.currentIndex < 0 || this.selection.currentIndex >= this.rowCount) {
      this.selection.currentIndex = this.rowCount - 1;
    }

    var accelKey = testAccelKey(event);

    if (event.keyCode == 13 && this.selection.count != 0) {                                     // enter
      if (this.selection.count == 1 && this.data[this.selection.currentIndex].isDirectory()) {  // if it's a directory
        if (!isReady()) {
          return;
        }
                                                                                                // navigate to it
        remoteDirTree.changeDir(this.data[this.selection.currentIndex].path);
      } else {
        if (gOpenMode) {
          this.launch();
        } else {
          new transfer().start(true);                                                           // else retrieve a file
        }
      }
    } else if (accelKey && (event.which == 65 || event.which == 97)) {
      event.preventDefault();                                                                   // accel-a: select all
      this.selection.selectAll();
    } else if (event.ctrlKey && event.keyCode == 32 && this.selection.count != 0) {             // ctrl-space, select or deselect
      this.selection.toggleSelect(this.selection.currentIndex);
    } else if (event.keyCode  == 8) {                                                           // backspace
      event.preventDefault();
      remoteDirTree.cdup();
    } else if (event.keyCode  == 116) {                                                         // F5
      event.preventDefault();
      this.refresh();
    } else if (event.keyCode  == 113 && this.selection.count != 0) {                            // F2
      this.rename();
    } else if (event.charCode == 100 && accelKey) {                                             // accel-d
      event.preventDefault();
      this.create(true);
    } else if (event.charCode == 110 && accelKey) {                                             // accel-n
      event.preventDefault();
      this.create(false);
    } else if (event.keyCode  == 46 && this.selection.count != 0) {                             // del
      this.remove();
    } else if (event.keyCode  == 93) {                                                          // display context menu
      var x = {};    var y = {};    var width = {};    var height = {};
      this.treebox.getCoordsForCellItem(this.selection.currentIndex, this.treebox.columns["remotename"], "text", x, y, width, height);
      $('remotemenu').showPopup(gRemoteTreeChildren, gRemoteTreeChildren.boxObject.x + 75, gRemoteTreeChildren.boxObject.y + y.value + 5, "context");
    } else if (event.charCode == 112 && accelKey && this.selection.count != 0) {                // accel-p
      event.preventDefault();
      this.showProperties(false);
    } else if (event.charCode == 120 && accelKey && this.selection.count != 0) {                // accel-x
      event.preventDefault();
      this.cut();
    } else if (event.charCode == 118 && accelKey) {                                             // accel-v
      event.preventDefault();
      this.paste();
    } else if (event.charCode == 111 && accelKey) {                                             // accel-o
      event.preventDefault();
      this.launch();
    } else if (event.charCode == 117 && accelKey) {                                             // accel-u
      event.preventDefault();
      this.copyUrl(true);
    }
  },

  // ************************************************* cut, copy, paste *****************************************************

  pasteFiles : new Array(),
  oldParent  : "",

  cut : function() {
    /*if (!gFtp.isConnected || this.selection.count == 0 || this.searchMode == 2) {
      return;
    }*/

    this.pasteFiles = new Array();
    this.oldParent  = gRemotePath.value;

    for (var x = 0; x < this.rowCount; ++x) {                                                   // put files to be cut/copied in an array to be pasted
      if (this.selection.isSelected(x)) {
        this.pasteFiles.push(this.data[x]);
        this.data[x].isCut = true;
        this.treebox.invalidateRow(x);
      }
    }

    $('remotePasteContext').setAttribute("disabled", false);                                    // enable pasting
  },

  paste : function(dest) {
    /*if (!gFtp.isConnected || this.pasteFiles.length == 0 || this.searchMode == 2) {
      return;
    }*/

    var newParent = dest ? dest          : gRemotePath.value;
    var files     = dest ? gFtp.listData : this.data;

    for (var x = 0; x < this.pasteFiles.length; ++x) {
      var newParentSlash = newParent               + (newParent.charAt(newParent.length - 1)                             != "/" ? "/" : '');
      var pasteFileSlash = this.pasteFiles[x].path + (this.pasteFiles[x].path.charAt(this.pasteFiles[x].path.length - 1) != "/" ? "/" : '');

      if (this.pasteFiles[x].isDirectory() && newParentSlash.indexOf(pasteFileSlash) == 0) {    // can't copy into a subdirectory of itself
        doAlert(gStrbundle.getString("copySubdirectory"));
        return;
      }
    }

    var prompt     = true;
    var skipAll    = false;
    var anyFolders = false;

    gFtp.beginCmdBatch();

    if (!dest) {
      gFtp.changeWorkingDirectory(newParent);
    }

    for (var x = 0; x < this.pasteFiles.length; ++x) {
      var newPath     = gFtp.constructPath(newParent, this.pasteFiles[x].leafName);
      var exists      = false;
      var isDirectory = false;
      var newFile;

      if (this.pasteFiles[x].isDirectory()) {
        anyFolders = true;
      }

      for (var y = 0; y < files.length; ++y) {
        if (files[y].leafName == this.pasteFiles[x].leafName) {
          exists      = true;
          newFile     = files[y];
          isDirectory = files[y].isDirectory();
          break;
        }
      }

      if (exists && skipAll) {
        continue;
      }

      if (exists && (isDirectory || this.pasteFiles[x].isDirectory())) {
        error(gStrbundle.getFormattedString("pasteErrorFile", [this.pasteFiles[x].path]));
        continue;
      }

      if (exists && prompt) {
        var params = { response         : 0,
                       fileName         : newPath,
                       resume           : true,
                       replaceResume    : true,
                       existingSize     : newFile.fileSize,
                       existingDate     : newFile.lastModifiedTime,
                       newSize          : this.pasteFiles[x].fileSize,
                       newDate          : this.pasteFiles[x].lastModifiedTime,
                       timerEnable      : false };

        window.openDialog("chrome://fireftp/content/confirmFile.xul", "confirmFile", "chrome,modal,dialog,resizable,centerscreen", params);

        if (params.response == 2) {
          prompt = false;
        } else if (params.response == 3) {
          continue;
        } else if (params.response == 4 || params.response == 0) {
          return;
        } else if (params.response == 5) {
          skipAll = true;
          continue;
        }
      }

      if (exists) {
        gFtp.remove(false, newPath);
      }

      var self          = this;
      var oldParent     = this.oldParent;
      var pasteCallback = function() { self.pasteCallback(oldParent, newParent, anyFolders, dest); };
      gFtp.rename(this.pasteFiles[x].path, newPath, x == this.pasteFiles.length - 1 && gRefreshMode ? pasteCallback : "", this.pasteFiles[x].isDirectory());
    }

    this.pasteFiles = new Array();
    $('remotePasteContext').setAttribute("disabled", true);

    gFtp.endCmdBatch();
  },

  pasteCallback : function(oldParent, newParent, anyFolders, dest) {
    remoteDirTree.addDirtyList(oldParent);
    remoteDirTree.addDirtyList(newParent);

    if (anyFolders) {
      var refreshIndex = dest ? remoteDirTree.indexOfPath(newParent) : remoteDirTree.indexOfPath(oldParent);

      if (refreshIndex != -1) {
        if (remoteDirTree.data[refreshIndex].open) {
          var self           = this;
          var pasteCallback2 = function() { self.pasteCallback2(oldParent, newParent, dest); };
          remoteDirTree.extraCallback = pasteCallback2;

          remoteDirTree.toggleOpenState(refreshIndex, true);                                       // close it up
          remoteDirTree.data[refreshIndex].children = null;                                        // reset its children
          remoteDirTree.toggleOpenState(refreshIndex);                                             // and open it up again
          return;
        } else {
          remoteDirTree.data[refreshIndex].children = null;                                        // reset its children
          remoteDirTree.data[refreshIndex].empty    = false;
          remoteDirTree.treebox.invalidateRow(refreshIndex);
        }

        var refreshIndex2 = dest ? remoteDirTree.indexOfPath(oldParent) : remoteDirTree.indexOfPath(newParent);

        if (refreshIndex2 == -1) {
          remoteDirTree.changeDir(dest ? oldParent : newParent);
          return;
        } else {
          remoteDirTree.selection.select(refreshIndex2);
        }
      }
    }

    this.refresh();
  },

  pasteCallback2 : function(oldParent, newParent, dest) {
    var refreshIndex2 = dest ? remoteDirTree.indexOfPath(oldParent) : remoteDirTree.indexOfPath(newParent);

    if (refreshIndex2 == -1) {
      remoteDirTree.changeDir(dest ? oldParent : newParent);
      return;
    } else {
      remoteDirTree.selection.select(refreshIndex2);
    }

    this.refresh();
  },

  canDrop : function(index, orient) {
    /*if (!gFtp.isConnected || !dragObserver.origin
      || (dragObserver.origin.indexOf('remote') != -1 && index == -1)
      || (dragObserver.origin.indexOf('remote') != -1 && !this.data[index].isDirectory())) {
      return false;
    }*/

    if (dragObserver.origin == 'remotetreechildren') {                                          // don't drag onto itself
      for (var x = 0; x < this.rowCount; ++x) {
        if (this.selection.isSelected(x) && index == x) {
          return false;
        }
      }
    }

    return true;
  },

  drop : function(index, orient) {
    if (dragObserver.origin == 'remotetreechildren') {
      this.cut();

      var self = this;
      var path = this.data[index].path;
      var func = function() { self.paste(path); };
      gFtp.list(this.data[index].path, func, true);
    } else if (dragObserver.origin == 'localtreechildren') {
      if (!dragObserver.overName || index == -1 || !this.data[index].isDirectory()) {
        new transfer().start(false);
      } else {
        var self                  = this;
        var path                  = this.data[index].path;
        var transferObj           = new transfer();
        transferObj.remoteRefresh = gRemotePath.value;
        var func                  = function() { transferObj.start(false, '', '', path); };
        gFtp.list(this.data[index].path, func, true);
      }
    } else if (dragObserver.origin == 'external') {
      var regular               = !dragObserver.overName || index == -1 || !this.data[index].isDirectory();
      var transferObj           = new transfer();
      transferObj.remoteRefresh = gRemotePath.value;

      for (var x = 0; x < dragObserver.externalFiles.length; ++x) {
        var droppedFile    = dragObserver.externalFiles[x];
        var fileParent     = droppedFile.parent ? droppedFile.parent.path : "";

        if (regular) {
          transferObj.start(false, droppedFile, fileParent, gRemotePath.value);
        } else {
          this.dropHelper(transferObj, droppedFile, fileParent, index);
        }

        if (transferObj.cancel) {
          break;
        }
      }
    }
  },

  dropHelper : function(transferObj, droppedFile, fileParent, index) {
    var self       = this;
    var remotePath = this.data[index].path;
    var func       = function() { transferObj.start(false, droppedFile, fileParent, remotePath); };
    gFtp.list(this.data[index].path, func, true);
  }
};
