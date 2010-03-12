function readPreferences(startup) {
  try {
    gDefaultAccount          = gPrefs.getComplexValue("defaultaccount", Components.interfaces.nsISupportsString).data;
    gBytesMode               = gPrefs.getBoolPref("bytesmode");
    gConcurrent              = gPrefs.getIntPref ("concurrent");
    gDebugMode               = gPrefs.getBoolPref("debugmode");
    gDisableDestructMode     = gPrefs.getBoolPref("destructmode");
    gDonated                 = gPrefs.getBoolPref("donated");
    gErrorMode               = gPrefs.getBoolPref("errormode");
    gInterfaceMode           = gPrefs.getIntPref ("interfacemode");
    gLogErrorMode            = gPrefs.getBoolPref("logerrormode");
    gLogMode                 = gPrefs.getBoolPref("logmode");
    gLogQueueMode            = gPrefs.getIntPref ("logqueue");
    gNoPromptMode            = gPrefs.getBoolPref("nopromptmode");
    gPasswordMode            = gPrefs.getBoolPref("passwordmode");
    gRefreshMode             = gPrefs.getBoolPref("refreshmode");
    gTempPasvMode            = gPrefs.getBoolPref("temppasvmode");
    gWelcomeMode             = gPrefs.getBoolPref("welcomemode");
    gOpenMode                = gPrefs.getIntPref ("openmode");

    gFireFTPUtils.hiddenMode = gPrefs.getBoolPref("hiddenmode");

    for (var x = 0; x < gMaxCon; ++x) {
      gConnections[x].fileMode            = gPrefs.getIntPref ("filemode");    // NOTE: if you add a preference here, don't forget to update fxp.js if needed
      gConnections[x].hiddenMode          = gPrefs.getBoolPref("hiddenmode");
      gConnections[x].keepAliveMode       = gPrefs.getBoolPref("keepalivemode");
      gConnections[x].networkTimeout      = gPrefs.getIntPref ("network");
      gConnections[x].proxyHost           = gPrefs.getComplexValue("proxyhost", Components.interfaces.nsISupportsString).data;
      gConnections[x].proxyPort           = gPrefs.getIntPref ("proxyport");
      gConnections[x].proxyType           = gPrefs.getCharPref("proxytype");
      gConnections[x].activePortMode      = gPrefs.getBoolPref("activeportmode");
      gConnections[x].activeLow           = gPrefs.getIntPref ("activelow");
      gConnections[x].activeHigh          = gPrefs.getIntPref ("activehigh");
      gConnections[x].reconnectAttempts   = gPrefs.getIntPref ("attempts");
      gConnections[x].reconnectInterval   = gPrefs.getIntPref ("retry");
      gConnections[x].reconnectMode       = gPrefs.getBoolPref("timeoutmode");
      gConnections[x].sessionsMode        = gPrefs.getBoolPref("sessionsmode");
      gConnections[x].timestampsMode      = gPrefs.getBoolPref("timestampsmode");
      gConnections[x].useCompression      = gPrefs.getBoolPref("compressmode");
      gConnections[x].integrityMode       = gPrefs.getBoolPref("integritymode");
    }

    if (gPrefs.getComplexValue("folder", Components.interfaces.nsISupportsString).data == "") {
      var file = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties)
                           .get("Home", Components.interfaces.nsILocalFile);

      var sString  = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
      if (file.path.indexOf('/') != -1) {
        sString.data = file.path.substring(0, file.path.indexOf('/') + 1);
      } else if (file.path.indexOf('\\') != -1) {
        sString.data = file.path.substring(0, file.path.indexOf('\\') + 1);
      }
      gPrefs.setComplexValue("folder", Components.interfaces.nsISupportsString, sString);
    }
/*
    if (gPrefs.getComplexValue("folder2", Components.interfaces.nsISupportsString).data == "") {
      var file2 = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties)
                           .get("Desk", Components.interfaces.nsILocalFile);

      var sString2  = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
      if (file2.path.indexOf('/') != -1) {
        sString2.data = file2.path.substring(0, file2.path.indexOf('/') + 1);
      } else if (file2.path.indexOf('\\') != -1) {
        sString2.data = file2.path.substring(0, file2.path.indexOf('\\') + 1);
      }
      gPrefs.setComplexValue("folder2", Components.interfaces.nsISupportsString, sString2);
    }
		*/
		

    if (startup) {
      gLocalPath.value = gPrefs.getComplexValue("folder", Components.interfaces.nsISupportsString).data;
      gRemotePath.value = gPrefs.getComplexValue("folder", Components.interfaces.nsISupportsString).data;
      gLoadUrl         = gPrefs.getComplexValue("loadurl", Components.interfaces.nsISupportsString).data;
    }

    updateInterface();
    updateOpenMode();

    $('logqueue').collapsed = !gLogMode;
    $('logsplitter').state  =  gLogMode ? 'open' : 'collapsed';
    $('logbutton').checked  =  gLogMode;

    $('logQueueTabs').selectedIndex = gLogQueueMode;

    $('statustype').label  = gTransferTypes[gFtp.fileMode];

    var asciiList = gPrefs.getComplexValue("asciifiles", Components.interfaces.nsISupportsString).data;
    asciiList     = asciiList.split(",");
    for (var x = 0; x < gMaxCon; ++x) {
      for (var y = 0; y < asciiList.length; ++y) {
        gConnections[x].asciiFiles.push(asciiList[y]);
      }
    }

  } catch (ex) {
    debug(ex);
  }
}

function showPreferences() {
  var branch       = gPrefsService.getBranch("browser.");
  var instantApply = branch.getBoolPref("preferences.instantApply");
  window.openDialog("chrome://fireftp/content/preferences.xul", "preferences", "chrome,resizable,centerscreen"
                                                                               + (instantApply ? ",dialog=no" : ",modal,dialog"));
}

var prefsObserver = {
  observe : function(prefsbranch, topic, data) {
    readPreferences();

    if (data == "fireftp.bytesmode") {
      localTree.updateView();

      if (gFtp.isConnected) {
        remoteTree.updateView();
      }
    } else if (data == "fireftp.logerrormode") {
      if (gLogErrorMode) {
        showOnlyErrors();
      } else {
        showAll();
      }
    } else if (data == "fireftp.hiddenmode") {
      if (!gFtp.hiddenMode) {
        var file        = localFile.init(gLocalPath.value);
        var hiddenFound = false;

        while (true) {
          if (file.isHidden() && file.path != localDirTree.data[0].path) {
            hiddenFound = true;
            break;
          }

          if (!(parent in file) || file.path == file.parent.path) {
            break;
          }

          file = file.parent;
        }

        if (hiddenFound) {
          gLocalPath.value = localDirTree.data[0].path;
        }
      }

      localDirTree.data     = new Array();
      localDirTree.treebox.rowCountChanged(0, -localDirTree.rowCount);
      localDirTree.rowCount = 0;
      localDirTree.changeDir(gLocalPath.value);

      if (gFtp.isConnected) {
        remoteTree.refresh();
      }
    }
  }
};
