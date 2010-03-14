function startup() {
  if (gStrbundle) {                            // we get two onload events b/c of the embedded browser
    return;
  }

  window.onerror         = detailedError;

  gStrbundle             = $("strings");
  gConnectButton         = $('connectbutton');
  gAccountField          = $('account');
  gFolderField           = $('folder');
  gFolderField2          = $('folder2');
  gLocalPath             = $('localpath');
  gLocalTree             = $('localtree');
  gLocalDirTree          = $('localdirtree');
  gLocalTreeChildren     = $('localtreechildren');
  gLocalDirTreeChildren  = $('localdirtreechildren');
  gRemotePath            = $('remotepath');
  gRemoteTree            = $('remotetree');
  gRemoteDirTree         = $('remotedirtree');
  gRemoteTreeChildren    = $('remotetreechildren');
  gRemoteDirTreeChildren = $('remotedirtreechildren');
  gCmdlogDoc             = $('cmdlog').contentWindow.document;
  gCmdlogBody            = $('cmdlog').contentWindow.document.body;
  gQueueTree             = $('queuetree');
  gQueueTreeChildren     = $('queuetreechildren');
  gStatusBytes           = $('statusbytes');
  gStatusElapsed         = $('statuselapsed');
  gStatusRemaining       = $('statusremaining');
  gStatusRate            = $('statusrate');
  gStatusMeter           = $('statusmeter');
  gLocalTree.view        = localTree;
  gLocalDirTree.view     = localDirTree;
  gRemoteTree.view       = localTree2;//remoteTree;
  gRemoteDirTree.view    = localDirTree2;//remoteDirTree;
  gQueueTree.view        = queueTree;

  gProfileDir            = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties)
                                     .get("ProfD", Components.interfaces.nsILocalFile);
  gAtomService           = Components.classes["@mozilla.org/atom-service;1"].getService            (Components.interfaces.nsIAtomService);
  gLoginManager          = Components.classes["@mozilla.org/login-manager;1"].getService           (Components.interfaces.nsILoginManager);
  gIos                   = Components.classes["@mozilla.org/network/io-service;1"].getService      (Components.interfaces.nsIIOService);
  gPromptService         = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);
  gPrefsService          = Components.classes["@mozilla.org/preferences-service;1"].getService     (Components.interfaces.nsIPrefService);
  gFireFTPUtils          = Components.classes['@nightlight.ws/fireftputils;1'].getService          (Components.interfaces.nsIFireFTPUtils);
  gFormHistory           = Components.classes["@mozilla.org/satchel/form-history;1"].getService    (Components.interfaces.nsIFormHistory ?
                                                                                                    Components.interfaces.nsIFormHistory :
                                                                                                    Components.interfaces.nsIFormHistory2);
  gLoginInfo             = new Components.Constructor("@mozilla.org/login-manager/loginInfo;1",     Components.interfaces.nsILoginInfo, "init");

  gPrefs                 = gPrefsService.getBranch("fireftp.");
  gPlatform              = getPlatform();

  if (gPrefsService instanceof Components.interfaces.nsIPrefBranchInternal) {
    gPrefsService.addObserver("fireftp", prefsObserver, false);
  }

  gMaxCon                = gPrefs.getIntPref("concurrentmax");

  gConnections           = new Array();
  for (var x = 0; x < gMaxCon; ++x) {
    gConnections.push(new ftpMozilla(x ? new transferObserver(x + 1) : ftpObserver));
    gConnections[x].type            = x ? 'transfer' : '';
    gConnections[x].connNo          = x + 1;
    gConnections[x].errorConnectStr = gStrbundle.getString("errorConn");
    gConnections[x].errorXCheckFail = gStrbundle.getString("errorXCheckFail");
    gConnections[x].passNotShown    = gStrbundle.getString("passNotShown");
    gConnections[x].l10nMonths      = gStrbundle.getString("months").split("|");
  }

  gFtp                   = gConnections[0];
  gFtp.setSecurity("");

  gTransferTypes         = new Array(gStrbundle.getString("auto"), gStrbundle.getString("binary"), gStrbundle.getString("ascii"));
  gMonths                = gStrbundle.getString("months").split("|");

  treeHighlighter.valid  = new Array({ tree: gLocalTree,  children: gLocalTreeChildren,  column: "localname"  },
                                     { tree: gRemoteTree, children: gRemoteTreeChildren, column: "remotename" },
                                     { tree: gQueueTree,  children: gQueueTreeChildren });

  if ($('searchWhich').selectedIndex == -1) {
    $('searchWhich').selectedIndex = 0;
  }


	searchSelectType();
	showSearchDates();
	securityPopup();
//commented-out stuff was causing startup to break, not defined, must have been pulled out??? --David

  readPreferences(true);
//  setConnectButton(true);
//  accountButtonsDisabler(true);
//  connectedButtonsDisabler();
  localDirTree.changeDir(gLocalPath.value);
// this was causing a non-fatal error...
  localDirTree2.changeDir(gRemotePath.value);
  //localDirTree2.changeDir(gLocalPath.value);

  //loadSiteManager(true);
  loadPrograms();

  gCmdlogBody.scrollTop = 0;

	//This was breaking too...
 // onAccountChange(gDefaultAccount);
  //setTimeout("gAccountField.focus()", 0);


  setTimeout(doResizeHack, 0);

  if (gLoadUrl) {
    setTimeout("externalLink()", 1000);
  }

	initCrypto();
	
}

function beforeUnload() {
  return "";
}

function unload() {
	finalizeCrypto();
  if (gPrefsService instanceof Components.interfaces.nsIPrefBranchInternal) {
    gPrefsService.removeObserver("fireftp", prefsObserver, false);
  }

  for (var x = 0; x < gMaxCon; ++x) {
    if (gConnections[x].isConnected) {
      gConnections[x].disconnect();
    }
  }

  if (gFxp && gFxp.isConnected) {
    gFxp.disconnect();
  }

  for (var x = 0; x < gTempEditFiles.length; ++x) {
    gFireFTPUtils.removeFile(gTempEditFiles[x].file);
  }
}
