function createAccount() {
  if (!gSiteManager.length) {
    newSite();
  }
}

function newSite() {
  var newSiteCallback = function(site) {
    gSiteManager.push(site);
    accountHelper(site);
  };

  var params = { callback    : newSiteCallback,
                 siteManager : gSiteManager,
                 localPath   : gLocalPath,
                 remotePath  : gRemotePath,
                 site        : { account  : "", host     : "",   port             : 21,    login          : "",    password : "",     anonymous : false,
                                 security : "", pasvmode : true, ipmode           : false, treesync       : false, localdir : "",     remotedir : "",
                                 webhost  : "", prefix   : "",   downloadcasemode : 0,     uploadcasemode : 0,     encoding : "UTF-8",
                                 notes    : "", timezone : 0,    folder           : "",    privatekey     : "" } };

  window.openDialog("chrome://fireftp/content/accountManager.xul", "accountManager", "chrome,dialog,resizable,centerscreen", params);
}

function editSite() {
  if (!gAccountField.value) {
    return;
  }

  var editSite;                                                      // grab a copy of the old site

  for (var x = 0; x < gSiteManager.length; ++x) {
    if (gSiteManager[x].account == gAccountField.value) {
      editSite = new cloneObject(gSiteManager[x]);
      break;
    }
  }

  var oldSite = new cloneObject(editSite);

  var editSiteCallback = function(site) {
    if (site.markedForDeath) {
      deleteSite(site);
      return;
    }

    for (var x = 0; x < gSiteManager.length; ++x) {
      if (gSiteManager[x].account == oldSite.account) {
        gSiteManager[x] = site;
        break;
      }
    }

    try {                                                            // delete old password from list
      var recordedHost = (oldSite.host.indexOf("ftp.") == 0 ? '' : "ftp.") + oldSite.host;
      var logins       = gLoginManager.findLogins({}, recordedHost, "FireFTP", null);
      for (var x = 0; x < logins.length; ++x) {
        if (logins[x].username == oldSite.login) {
          gLoginManager.removeLogin(logins[x]);
        }
      }
    } catch (ex) { }

    accountHelper(site);
  };

  var params          = { callback    : editSiteCallback,
                          siteManager : gSiteManager,
                          localPath   : gLocalPath,
                          remotePath  : gRemotePath,
                          site        : editSite };

  window.openDialog("chrome://fireftp/content/accountManager.xul", "accountManager", "chrome,dialog,resizable,centerscreen", params);
}

function deleteSite(site) {
  for (var x = 0; x < gSiteManager.length; ++x) {                    // delete the account
    if (gSiteManager[x].account == site.account) {
      try {                                                          // delete password from list
        var recordedHost = (gSiteManager[x].host.indexOf("ftp.") == 0 ? '' : "ftp.") + gSiteManager[x].host;
        var logins       = gLoginManager.findLogins({}, recordedHost, "FireFTP", null);
        for (var y = 0; y < logins.length; ++y) {
          if (logins[y].username == gSiteManager[x].login) {
            gLoginManager.removeLogin(logins[y]);
          }
        }
      } catch (ex) { }

      gSiteManager.splice(x, 1);

      break;
    }
  }

  saveSiteManager();
  loadSiteManager();

  onFolderChange();
}

function quickConnect() {                                            // make a quick connection, account not saved
  var quickConnectCallback = function(site) {
    tempAccountHelper(site);
  };

  var quickConnectCancelCallback = function() {
    $('quickMenuItem').setAttribute("disabled", gFtp.isConnected);
  };

  $('quickMenuItem').setAttribute("disabled", true);

  var params = { callback       : quickConnectCallback,
                 cancelCallback : quickConnectCancelCallback,
                 siteManager    : gSiteManager,
                 quickConnect   : true,
                 localPath      : gLocalPath,
                 remotePath     : gRemotePath,
                 site           : { account  : "", host     : "",   port             : 21,    login          : "",    password : "",     anonymous : false,
                                    security : "", pasvmode : true, ipmode           : false, treesync       : false, localdir : "",     remotedir : "",
                                    webhost  : "", prefix   : "",   downloadcasemode : 0,     uploadcasemode : 0,     encoding : "UTF-8",
                                    notes    : "", timezone : 0,    folder           : "",    privatekey     : "",
                                    temporary : true } };

  window.openDialog("chrome://fireftp/content/accountManager.xul", "accountManager", "chrome,dialog,resizable,centerscreen", params);
}

function externalLink() {                                            // opened up fireFTP using a link in Firefox
  var site = { account  : "", host     : "",            port             : 21,    login          : "anonymous", password : "fireftp@example.com", anonymous : true,
               security : "", pasvmode : gTempPasvMode, ipmode           : false, treesync       : false,       localdir : "",                    remotedir : "",
               webhost  : "", prefix   : "",            downloadcasemode : 0,     uploadcasemode : 0,           encoding : "UTF-8",
               notes    : "", timezone : 0,             folder           : "",    privatekey     : "",
               temporary : true };

  var uri    = Components.classes["@mozilla.org/network/standard-url;1"].getService(Components.interfaces.nsIURI);
  var toUTF8 = Components.classes["@mozilla.org/intl/utf8converterservice;1"].getService(Components.interfaces.nsIUTF8ConverterService);
  uri.spec   = gLoadUrl;

  if (!uri.schemeIs("ftp") || gLoadUrl.length <= 6) {                // sanity check
    return;
  }

  if (uri.username) {
    site.login     = unescape(uri.username);
    site.password  = unescape(uri.password);
    site.anonymous = site.login ? false : true;
  }

  if (uri.username && !uri.password) {
    try {
      var logins = gLoginManager.findLogins({}, 'ftp://' + site.login + '@' + uri.host, "FireFTP", null);
      for (var x = 0; x < logins.length; ++x) {
        if (logins[x].username == site.login) {
          site.password = logins[x].password;
          break;
        }
      }
    } catch (ex) { }
  }

  if (!uri.username && !uri.password) {
    try {
      var logins = gLoginManager.findLogins({}, 'ftp://' + uri.host, "FireFTP", null);
      for (var x = 0; x < logins.length; ++x) {
        site.login    = logins[x].username;
        site.password = logins[x].password;
        break;
      }
    } catch (ex) { }
  }

  site.host = uri.host;
  site.port = uri.port == -1 ? 21 : uri.port;

  var prefBranch   = gPrefsService.getBranch("browser.");
  gLoadUrl         = uri.path.charAt(uri.path.length - 1) == '/' ? "" : unescape(uri.path);

  try {
    gLoadUrl       = toUTF8.convertStringToUTF8(gLoadUrl, "UTF-8", 1);
  } catch (ex) {
    debug(ex);
  }

  try {
    if (prefBranch.getBoolPref("download.useDownloadDir")) {
      site.localdir  = prefBranch.getComplexValue("download.dir", Components.interfaces.nsISupportsString).data;
    }
  } catch (ex) { }

  site.remotedir = gLoadUrl == "" ? unescape(uri.path) : unescape(uri.path.substring(0, uri.path.lastIndexOf('/')));

  try {
    site.remotedir = toUTF8.convertStringToUTF8(site.remotedir, "UTF-8", 1);
  } catch (ex) {
    debug(ex);
  }

  gPrefs.setCharPref("loadurl", "");

  tempAccountHelper(site);
}

function accountHelper(site) {
  if (gPasswordMode && site.password) {
    try {                                                            // save username & password
      var recordedHost = (site.host.indexOf("ftp.") == 0 ? '' : "ftp.") + site.host;
      var loginInfo    = new gLoginInfo(recordedHost, "FireFTP", null, site.login, site.password, "", "");
      gLoginManager.addLogin(loginInfo);
    } catch (ex) { }
  }

  var tempPassword = site.password;
  saveSiteManager();                                                 // save site manager
  loadSiteManager();

  for (var x = 0; x < gSiteManager.length; ++x) {                    // select the new site
    if (gSiteManager[x].account == site.account) {
      gAccountField.selectedIndex = x;
      gSiteManager[x].password    = tempPassword;                    // if "Remember Passwords" is off we have to remember what it is temporarily
      onAccountChange(site.account);
      break;
    }
  }
}

function tempAccountHelper(site) {
  site.account = site.host;

  var found = true;
  var count = 0;

  while (found) {
    found = false;

    for (var x = 0; x < gSiteManager.length; ++x) {
      if (gSiteManager[x].account == site.account) {
        found = true;
        ++count;
        site.account = site.host + '-' + count.toString();
        break;
      }
    }
  }

  gSiteManager.push(site);

  accountHelper(site);

  connect(true);
}

function onFolderChange(dontSelect, click) {
  if (click && gFolder == gFolderField.value) {
    return;
  }

  gAccountField.removeAllItems();

  if (!gSiteManager.length) {
    gAccountField.setAttribute("label", gStrbundle.getString("createAccount"));
  }

  gAccountField.appendItem(gStrbundle.getString("createAccount"), "");
  gAccountField.firstChild.lastChild.setAttribute("oncommand", "newSite()");
  gAccountField.appendItem(gStrbundle.getString("quickConnectMenu"), "");
  gAccountField.firstChild.lastChild.setAttribute("id", "quickMenuItem");
  gAccountField.firstChild.lastChild.setAttribute("oncommand", "quickConnect()");

  if (gSiteManager.length) {
    gAccountField.firstChild.appendChild(document.createElement("menuseparator"));
  }

  for (var x = 0; x < gSiteManager.length; ++x) {
    if (gSiteManager[x].folder == gFolderField.value || (!gSiteManager[x].folder && gFolderField.value == "")) {
      gAccountField.appendItem(gSiteManager[x].account, gSiteManager[x].account);
    }
  }

  if (!dontSelect && gSiteManager.length) {
    gAccountField.selectedIndex = 3;
    onAccountChange();
  }

  gFolder = gFolderField.value;
}

function onAccountChange(account) {
  if (account != null) {
    var found = -1;

    for (var x = 0; x < gSiteManager.length; ++x) {
      if (gSiteManager[x].account == account) {
        found = x;
        break;
      }
    }

    if (found == -1) {
      gFolderField.value = "";
      onFolderChange();
      return;
    }

    gFolderField.value  = gSiteManager[x].folder;
    onFolderChange(true);
    gAccountField.value = account;
  }

  var accountToLoad = gFtp.isConnected ? gAccount : gAccountField.value;

  for (var x = 0; x < gSiteManager.length; ++x) {                    // load up the new values into the global variables
    if (gSiteManager[x].account == accountToLoad) {
      accountChangeHelper(gSiteManager[x]);
      break;
    }
  }

  if (gAccountField.value) {
    accountButtonsDisabler(false);
  }
}

var gDidPuttyCredit = false;
function accountChangeHelper(site) {
  if (!gFtp.isConnected) {
    gFtp.setSecurity(site.security);

    if (site.security == "sftp" && !gDidPuttyCredit) {
      appendLog(gStrbundle.getFormattedString("putty", ["0.60"])
        + "<br style='font-size:5pt'/><br style='font-size:5pt'/>", 'blue', "info");
      gDidPuttyCredit = true;
    }
  }

  for (var x = 0; x < gMaxCon; ++x) {
    if (!gFtp.isConnected) {
      gConnections[x].host       = site.host;
      gConnections[x].port       = site.port;
      gConnections[x].login      = site.login;
      gConnections[x].password   = site.password;
      gConnections[x].privatekey = site.privatekey;
    }

    gConnections[x].passiveMode  = site.pasvmode;
    gConnections[x].ipType       = site.ipmode ? "IPv6" : "IPv4";
    gConnections[x].setEncoding   (site.encoding || "UTF-8");
    gConnections[x].initialPath  = site.remotedir ? site.remotedir : '';
    gConnections[x].timezone     = site.timezone  ? site.timezone  : 0;
  }

  gAccount          = site.account;
  gDownloadCaseMode = site.downloadcasemode;
  gUploadCaseMode   = site.uploadcasemode;
  gWebHost          = site.webhost;
  gPrefix           = site.prefix;
  gTreeSync         = site.treesync;
  gTreeSyncLocal    = site.localdir;
  gTreeSyncRemote   = site.remotedir;

  if (!gFtp.isConnected) {
    if (site.localdir) {
      var dir = localFile.init(site.localdir);
      if (localFile.verifyExists(dir)) {
        localDirTree.changeDir(site.localdir);
      } else {
        error(gStrbundle.getString("noPermission"));
      }
    }

    if (site.remotedir) {
      gRemotePath.value = site.remotedir;
    } else {
      gRemotePath.value = "/";
    }
  }

  if (site.account) {
    var sString  = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
    sString.data = site.account;
    gPrefs.setComplexValue("defaultaccount", Components.interfaces.nsISupportsString, sString);
  }

  accountButtonsDisabler(false);
}

function setConnectButton(connect) {
  gConnectButton.label =                   connect ? gStrbundle.getString("connectButton") : gStrbundle.getString("disconnectButton");
  gConnectButton.setAttribute('command',   connect ? 'cmd_connect'                         : 'cmd_disconnect');
  gConnectButton.setAttribute('accesskey', connect ? gStrbundle.getString("connectAccess") : gStrbundle.getString("disconnectAccess"));
}

function accountButtonsDisabler(enable) {
  $('editMenuItem').setAttribute(  "disabled", enable);

  if (!gFtp.isConnected) {
    $('connectbutton').disabled = enable;
  }
}

function connectedButtonsDisabler() {
  $('abortbutton').disabled =                             !gFtp.isConnected;
  $('retrieveButton').disabled =                          !gFtp.isConnected;
  $('storeButton').disabled =                             !gFtp.isConnected;
  $('remoteUpButton').disabled =                          !gFtp.isConnected;
  $('remoteRefreshButton').disabled =                     !gFtp.isConnected;
  $('remoteChangeButton').disabled =                      !gFtp.isConnected;
  $('searchRemote').disabled =                            !gFtp.isConnected;
  $('diffMenuItem').setAttribute(             "disabled", !gFtp.isConnected);
  $('recDiffMenuItem').setAttribute(          "disabled", !gFtp.isConnected);
  $('customMenuItem').setAttribute(           "disabled", !gFtp.isConnected);
  $('localUpload').setAttribute(              "disabled", !gFtp.isConnected);
  $('remoteOpenCont').setAttribute(           "disabled", !gFtp.isConnected);
  $('remoteDownload').setAttribute(           "disabled", !gFtp.isConnected);
  $('remoteOpen').setAttribute(               "disabled", !gFtp.isConnected);
  $('remoteOpenWith').setAttribute(           "disabled", !gFtp.isConnected);
  $('remoteWeb').setAttribute(                "disabled", !gFtp.isConnected);
  $('remoteCopyUrl').setAttribute(            "disabled", !gFtp.isConnected);
  $('remoteFXP').setAttribute(                "disabled", !gFtp.isConnected || gFtp.security == "sftp");
  $('remoteCutContext').setAttribute(         "disabled", !gFtp.isConnected);
  $('remotePasteContext').setAttribute(       "disabled", !gFtp.isConnected);
  $('remoteCreateDir').setAttribute(          "disabled", !gFtp.isConnected);
  $('remoteCreateFile').setAttribute(         "disabled", !gFtp.isConnected);
  $('remoteRemove').setAttribute(             "disabled", !gFtp.isConnected);
  $('remoteRename').setAttribute(             "disabled", !gFtp.isConnected);
  $('remoteProperties').setAttribute(         "disabled", !gFtp.isConnected);
  $('remoteRecursiveProperties').setAttribute("disabled", !gFtp.isConnected);
  $('queueRetry').setAttribute(               "disabled", !gFtp.isConnected);
  $('queueCancel').setAttribute(              "disabled", !gFtp.isConnected);
  $('queueCancelAll').setAttribute(           "disabled", !gFtp.isConnected);
  if ($('quickMenuItem')) {
    $('quickMenuItem').setAttribute(          "disabled",  gFtp.isConnected);
  }
  $('remotepath').setAttribute("disconnected",            !gFtp.isConnected);
  remoteDirTree.treebox.invalidate();
  remoteTree.treebox.invalidate();

  searchSelectType();
}

function loadSiteManager(pruneTemp, importFile) {             // read gSiteManager data
  try {
    gFolderField.removeAllItems();

    if (!gFtp.isConnected) {
      gFtp.setSecurity("");

      for (var x = 0; x < gMaxCon; ++x) {
        gConnections[x].host         = "";
        gConnections[x].port         = 21;
        gConnections[x].login        = "";
        gConnections[x].password     = "";
        gConnections[x].passiveMode  = true;
        gConnections[x].initialPath  = "";
        gConnections[x].setEncoding("UTF-8");
        gConnections[x].timezone     = 0;
        gConnections[x].privatekey   = "";
      }

      gAccount          = "";
      gDownloadCaseMode = 0;
      gUploadCaseMode   = 0;
      gWebHost          = "";
      gPrefix           = "";
      gRemotePath.value = "/";
    }

    var file;
    if (importFile) {
      file = importFile;
    } else {
      file = gProfileDir.clone();
      file.append("fireFTPsites.dat");
    }

    var folders = new Array();
    if (!file.exists() && !importFile) {
      gSiteManager = new Array();
    } else if (file.exists()) {
      var fstream  = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
      var sstream  = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
      fstream.init(file, 1, 0, false);
      sstream.init(fstream);

      var siteData = "";
      var str      = sstream.read(-1);

      while (str.length > 0) {
        siteData += str;
        str       = sstream.read(-1);
      }

      if (localTree.getExtension(file.leafName) == "xml") {
        siteData = importFileZilla(siteData);
        siteData = siteData.toSource();
      }

      if (importFile) {
        try {
          var tempSiteManager = eval(siteData);
        } catch (ex) {
          error(gStrbundle.getString("badImport"));
          sstream.close();
          fstream.close();
          return;
        }

        var passCheck = false;
        var toUTF8    = Components.classes["@mozilla.org/intl/utf8converterservice;1"].getService(Components.interfaces.nsIUTF8ConverterService);
        var key;
        for (var x = 0; x < tempSiteManager.length; ++x) {
          if (tempSiteManager[x].passcheck) {
            passCheck = true;
            var passwordObject       = new Object();
            passwordObject.returnVal = false;

            window.openDialog("chrome://fireftp/content/password2.xul", "password", "chrome,modal,dialog,resizable,centerscreen", passwordObject);

            if (passwordObject.returnVal) {
              key = passwordObject.password;
            } else {
              sstream.close();
              fstream.close();
              return;
            }

            key = key ? key : "";
            if (rc4Decrypt(key, tempSiteManager[x].password) != "check123") {
              error(gStrbundle.getString("badPassword"));
              sstream.close();
              fstream.close();
              return;
            }
            break;
          }
        }

        for (var x = 0; x < tempSiteManager.length; ++x) {
          if (tempSiteManager[x].passcheck) {
            continue;
          }

          var found   = true;
          var count   = 0;
          var skip    = true;
          var account = tempSiteManager[x].account;

          while (found) {
            found = false;

            for (var y = 0; y < gSiteManager.length; ++y) {
              if (gSiteManager[y].account == account) {
                found = true;

                for (i in gSiteManager[y]) {                         // if it's the exact same object skip it
                  if (i != "password" && gSiteManager[y][i] != tempSiteManager[x][i]) {
                    skip = false;
                    break;
                  }
                }

                if (skip) {
                  break;
                }

                ++count;
                account = tempSiteManager[x].account + '-' + count.toString();
                break;
              }
            }

            if (skip) {
              break;
            }
          }

          if (skip && found) {
            continue;
          }

          if ((gSlash == "/" && tempSiteManager[x].localdir.indexOf("/") == -1) || (gSlash == "\\" && tempSiteManager[x].localdir.indexOf("\\") == -1)) {
            tempSiteManager[x].localdir = "";
            tempSiteManager[x].treesync = false;
          }

          if (passCheck) {
            tempSiteManager[x].password = rc4Decrypt(key, tempSiteManager[x].password);

            try {
              tempSiteManager[x].password = toUTF8.convertStringToUTF8(tempSiteManager[x].password, "UTF-8", 1);
            } catch (ex) {
              debug(ex);
            }
          }

          if (gPasswordMode && tempSiteManager[x].password) {
            try {                                                    // save username & password
              var recordedHost = (tempSiteManager[x].host.indexOf("ftp.") == 0 ? '' : "ftp.") + tempSiteManager[x].host;
              var loginInfo    = new gLoginInfo(recordedHost, "FireFTP", null, tempSiteManager[x].login, tempSiteManager[x].password, "", "");
              gLoginManager.addLogin(loginInfo);
            } catch (ex) { }
          }

          tempSiteManager[x].account = account;
          gSiteManager.push(tempSiteManager[x]);
        }
      } else {
        gSiteManager = eval(siteData);
      }

      if (gPasswordMode) {
        for (var x = 0; x < gSiteManager.length; ++x) {              // retrieve passwords from passwordmanager
          try {
            var logins = gLoginManager.findLogins({}, (gSiteManager[x].host.indexOf("ftp.") == 0 ? '' : "ftp.") + gSiteManager[x].host, "FireFTP", null);
            var found  = false;
            for (var y = 0; y < logins.length; ++y) {
              if (logins[y].username == gSiteManager[x].login) {
                gSiteManager[x].password = logins[y].password;
                found = true;
                break;
              }
            }
            if (!found) {                                            // firefox 2 -> 3 growing pains, yay...
              var logins = gLoginManager.findLogins({}, 'http://' + (gSiteManager[x].host.indexOf("ftp.") == 0 ? '' : "ftp.") + gSiteManager[x].host, "FireFTP", null);
              for (var y = 0; y < logins.length; ++y) {
                if (logins[y].username == gSiteManager[x].login) {
                  gSiteManager[x].password = logins[y].password;
                  found = true;
                  break;
                }
              }
            }
            if (!found) {                                            // firefox 2 -> 3 growing pains, yay...
              var logins = gLoginManager.findLogins({}, (gSiteManager[x].host.indexOf("ftp.") == 0 ? '' : "ftp.") + gSiteManager[x].host, null,
                                                        (gSiteManager[x].host.indexOf("ftp.") == 0 ? '' : "ftp.") + gSiteManager[x].host);
              for (var y = 0; y < logins.length; ++y) {
                if (logins[y].username == gSiteManager[x].login) {
                  gSiteManager[x].password = logins[y].password;
                  break;
                }
              }
            }
          } catch (ex) { }
        }
      }

      sstream.close();
      fstream.close();

      if (pruneTemp) {
        for (var x = gSiteManager.length - 1; x >= 0; --x) {
          if (gSiteManager[x].temporary) {
            try {                                                    // delete password from list
              var recordedHost = (gSiteManager[x].host.indexOf("ftp.") == 0 ? '' : "ftp.") + gSiteManager[x].host;
              var logins       = gLoginManager.findLogins({}, recordedHost, "FireFTP", null);
              for (var y = 0; y < logins.length; ++y) {
                if (logins[y].username == gSiteManager[x].login) {
                  gLoginManager.removeLogin(logins[y]);
                }
              }
            } catch (ex) { }

            gSiteManager.splice(x, 1);
          }
        }
      }

      for (var x = 0; x < gSiteManager.length; ++x) {
        var found = false;
        gSiteManager[x].folder = gSiteManager[x].folder || "";

        for (var y = 0; y < folders.length; ++y) {
          if (gSiteManager[x].folder == folders[y]) {
            found = true;
            break;
          }
        }

        if (!found) {
          folders.push(gSiteManager[x].folder);
        }
      }

      folders.sort();

      for (var x = 0; x < folders.length; ++x) {
        gFolderField.appendItem(folders[x] ? folders[x] : gStrbundle.getString("noFolder"), folders[x]);
      }
    }

    if (!folders.length) {
      gFolderField.appendItem(gStrbundle.getString("noFolder"), "");
    }

    gFolderField.selectedIndex = 0;
    $('folderItem').collapsed = !folders.length || (folders.length == 1 && folders[0] == "");

    if (gSiteManager.length) {
      gAccountField.setAttribute("label", gStrbundle.getString("chooseAccount"));
    } else {
      gAccountField.setAttribute("label", gStrbundle.getString("createAccount"));
    }

    accountButtonsDisabler(true);
  } catch (ex) {
    debug(ex);
  }
}

function saveSiteManager(exportFile) {
  try {                                                              // write gSiteManager out to disk
    var tempSiteManagerArray = new Array();

    for (var x = 0; x < gSiteManager.length; ++x) {
      tempSiteManagerArray.push(new cloneObject(gSiteManager[x]));
    }

    var key;
    if (exportFile) {
      var passwordObject       = new Object();
      passwordObject.returnVal = false;

      window.openDialog("chrome://fireftp/content/password2.xul", "password", "chrome,modal,dialog,resizable,centerscreen", passwordObject);

      if (passwordObject.returnVal) {
        key = passwordObject.password;
      } else {
        return;
      }

      key = key ? key : "";
      tempSiteManagerArray.push({ account: "a", passcheck: "check123", password: "check123" });
    }

    var fromUTF8     = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"].getService(Components.interfaces.nsIScriptableUnicodeConverter);
    fromUTF8.charset = "UTF-8";
    for (var x = 0; x < tempSiteManagerArray.length; ++x) {          // we don't save out the passwords, those are saved in the passwordmanager
      if (exportFile) {
        try {
          tempSiteManagerArray[x].password = fromUTF8.ConvertFromUnicode(tempSiteManagerArray[x].password) + fromUTF8.Finish();
        } catch (ex) {
          debug(ex);
        }

        tempSiteManagerArray[x].password = rc4Encrypt(key, tempSiteManagerArray[x].password);
      } else {
        tempSiteManagerArray[x].password = "";
      }
    }

    var file;
    if (exportFile) {
      file = exportFile;
    } else {
      file = gProfileDir.clone();
      file.append("fireFTPsites.dat");
    }

    var foutstream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
    foutstream.init(file, 0x04 | 0x08 | 0x20, 0644, 0);
    tempSiteManagerArray.sort(compareAccount);
    foutstream.write(tempSiteManagerArray.toSource(), tempSiteManagerArray.toSource().length);
    foutstream.close();
  } catch (ex) {
    debug(ex);
  }
}

function importSites() {
  var nsIFilePicker   = Components.interfaces.nsIFilePicker;
  var fp              = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
  fp.defaultExtension = "dat";
  fp.appendFilter("FireFTP (*.dat)", "*.dat");
  fp.appendFilter("FileZilla (*.xml)", "*.xml");
  fp.init(window, null, nsIFilePicker.modeOpen);
  var res = fp.show();

  if (res != nsIFilePicker.returnOK) {
    return;
  }

  var tempAccount = gAccountField.value;

  loadSiteManager(true, fp.file);
  saveSiteManager();                                                 // save site manager
  loadSiteManager();

  onAccountChange(tempAccount);                                      // select the new site
}

function importFileZilla(theString) {
  var parser    = new DOMParser();
  var dom       = parser.parseFromString(theString, "text/xml");

  if (dom.documentElement.nodeName == "parsererror") {
    error(gStrbundle.getString("badImport"));
    return new Array();
  }

  var el        = dom.firstChild.firstChild;
  while (el) {                                                       // find the servers or sites element
    if (el.nodeName == 'Sites' || el.nodeName == 'Servers') {
      el = el.firstChild;
      break;
    }

    el = el.nextSibling;
  }

  return fileZillaHelper(el);
}

function fileZillaHelper(el, folder) {
  var siteData = new Array();

  while (el) {                                                       // find the server or site element
    if (el.nodeName == 'Folder') {
      var newFolder  = (folder ? folder + "-" : "") + (el.getAttribute('Name') ? el.getAttribute('Name') : trim(el.firstChild.nodeValue));
      var returnData = fileZillaHelper(el.firstChild, newFolder);

      for (var x = 0; x < returnData.length; ++x) {
        siteData.push(returnData[x]);
      }
    } else if (el.nodeName == 'Site') {                              // filezilla 2
      var password = "";

      if (el.getAttribute('Pass')) {
        var cipher  = "FILEZILLA1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        var pass    = el.getAttribute('Pass');
        var passLen = parseInt(pass.length / 3);
        var offset  = (pass.length / 3) % cipher.length;

        for (var x = 0; x < pass.length; x += 3) {
          var c1 = parseInt(pass.substring(x, x + 3), 10);
          var c2 = cipher.charCodeAt((x / 3 + offset) % cipher.length);
          var c3 = String.fromCharCode(c1 ^ c2);
          password += c3;
        }
      }

      var obj = {
        account           : el.getAttribute('Name'),
        folder            : folder ? folder : "",
        host              : el.getAttribute('Host'),
        port              : el.getAttribute('Port'),
        login             : el.getAttribute('Logontype') == "0" ? "anonymous"           : el.getAttribute('User'),
        password          : el.getAttribute('Logontype') == "0" ? "fireftp@example.com" : password,
        anonymous         : el.getAttribute('Logontype') == "0",
        security          : el.getAttribute('ServerType') == "1" ? "ssl" : (el.getAttribute('ServerType') == "2" ? "authssl" : (el.getAttribute('ServerType') == "3" ? "sftp" : (el.getAttribute('ServerType') == "4" ? "authtls" : ""))),
        pasvmode          : el.getAttribute('PasvMode') != "2",
        localdir          : el.getAttribute('LocalDir'),
        remotedir         : el.getAttribute('RemoteDir'),
        notes             : el.getAttribute('Comments'),
        timezone          : (parseInt(el.getAttribute('TimeZoneOffset')) * 60) + parseInt(el.getAttribute('TimeZoneOffsetMinutes')),
        ipmode            : false,
        treesync          : false,
        webhost           : "",
        privatekey        : "",
        prefix            : "",
        encoding          : "UTF-8",
        downloadcasemode  : 0,
        uploadcasemode    : 0
      };

      siteData.push(obj);
    } else if (el.nodeName == 'Server') {                            // filezilla 3
      var serverEl = el.firstChild;

      var obj = { account  : "", host     : "",   port             : 21,    login          : "",    password : "",     anonymous : false,
                  security : "", pasvmode : true, ipmode           : false, treesync       : false, localdir : "",     remotedir : "",
                  webhost  : "", prefix   : "",   downloadcasemode : 0,     uploadcasemode : 0,     encoding : "UTF-8",
                  notes    : "", timezone : 0,    folder           : "",    privatekey     : "" };
      obj.account = trim(el.lastChild.nodeValue);
      obj.folder  = folder ? folder : "";

      while (serverEl) {
        switch (serverEl.nodeName) {
          case "Host":
            obj.host      = serverEl.textContent;
            break;
          case "Port":
            obj.port      = serverEl.textContent;
            break;
          case "Protocol":
            obj.security  = serverEl.textContent == "3" ? "authssl" : (serverEl.textContent == "4" ? "authtls" : (serverEl.textContent == "1" ? "sftp" : ""));
            break;
          case "Logontype":
            obj.anonymous = serverEl.textContent == "0";
            break;
          case "User":
            obj.login     = serverEl.textContent;
            break;
          case "Pass":
            obj.password  = serverEl.textContent;
            break;
          case "TimezoneOffset":
            obj.timezone  = serverEl.textContent;
            break;
          case "PasvMode":
            obj.pasvmode  = serverEl.textContent != "MODE_ACTIVE";
            break;
          case "Comments":
            obj.notes     = serverEl.textContent;
            break;
          case "LocalDir":
            obj.localdir  = serverEl.textContent;
            break;
          case "RemoteDir":
            obj.remotedir = serverEl.textContent.substring(3);       // seriously, wtf? example: "1 0 4 test 4 aeou 7 aoe4aoe 2 4 "
            obj.remotedir = obj.remotedir.replace(/\s[0-9]\s/g, "/");
            break;
          default:
            break;
        }

        serverEl = serverEl.nextSibling;
      }

      if (obj.anonymous) {
        obj.login    = "anonymous";
        obj.password = "fireftp@example.com";
      }

      siteData.push(obj);
    }

    el = el.nextSibling;
  }

  return siteData;
}

function exportSites() {
  var nsIFilePicker   = Components.interfaces.nsIFilePicker;
  var fp              = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
  fp.defaultString    = "fireFTPsites.dat";
  fp.defaultExtension = "dat";
  fp.appendFilter("FireFTP (*.dat)", "*.dat");
  fp.init(window, null, nsIFilePicker.modeSave);
  var res = fp.show();

  if (res == nsIFilePicker.returnOK || res == nsIFilePicker.returnReplace) {
    saveSiteManager(fp.file);
  }
}

/* RC4 symmetric cipher encryption/decryption
 * Copyright (c) 2006 by Ali Farhadi.
 * released under the terms of the Gnu Public License.
 * see the GPL for details.
 *
 * Email: ali[at]farhadi[dot]ir
 * Website: http://farhadi.ir/
 */

function rc4Encrypt(key, pt) {
	s = new Array();

	for (var i = 0; i < 256; ++i) {
		s[i] = i;
	}

	var j = 0;
	var x;

	for (i = 0; i < 256; ++i) {
		j    = (j + s[i] + key.charCodeAt(i % key.length)) % 256;
		x    = s[i];
		s[i] = s[j];
		s[j] = x;
	}

	i = 0;
	j = 0;
	var ct = '';

	for (var y = 0; y < pt.length; ++y) {
		i    = (i + 1)    % 256;
		j    = (j + s[i]) % 256;
		x    = s[i];
		s[i] = s[j];
		s[j] = x;
		ct  += String.fromCharCode(pt.charCodeAt(y) ^ s[(s[i] + s[j]) % 256]);
	}

	return ct;
}

function rc4Decrypt(key, ct) {
	return rc4Encrypt(key, ct);
}
