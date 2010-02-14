function commas(num) {                 // add commas to numbers
  num = num.toString();

  if (num.search(/\d{4}$/) == -1 || !gStrbundle.getString("numSep")) {
    return num;
  }

  num = num.replace(/\d{3}$/, gStrbundle.getString("numSep") + "$&");

  var a = new RegExp("\\d{4}" + (gStrbundle.getString("numSep") == "." ? "\\." : gStrbundle.getString("numSep")));
  var b = new RegExp("\\d{3}" + (gStrbundle.getString("numSep") == "." ? "\\." : gStrbundle.getString("numSep")));

  while (num.search(a) != -1) {
    num = num.replace(b, gStrbundle.getString("numSep") + "$&");
  }

  return num;
}

function zeros(num) {                  // pad with zeros
  num = num.toString();
  return num.length == 2 ? num : '0' + num;
}

function onChangeType() {              // change TYPE A/TYPE I/auto from statusbar
  gPrefs.setIntPref("filemode", (gFtp.fileMode + 1) % 3);
}

function setInterfaceMode() {          // update the interface based on collapsing
  gPrefs.setIntPref("interfacemode", ($('leftsplitter').getAttribute('state')  == 'collapsed') * 2
                                   + ($('rightsplitter').getAttribute('state') == 'collapsed'));
}

function updateInterface() {           // update the interface based on interfacemode variable
  var local  = (gInterfaceMode & 2);
  var remote = (gInterfaceMode & 1);

  $('storbutton').collapsed  = local;
  $('retrbutton').collapsed  = remote;

  $('localview').collapsed   = local;
  $('remoteview').collapsed  = remote;

  $('leftsplitter').setAttribute( 'state', (local  ? 'collapsed' : 'open'));
  $('rightsplitter').setAttribute('state', (remote ? 'collapsed' : 'open'));
}

function updateOpenMode() {
  $('localUpload').setAttribute(   "defaultAction", gOpenMode == 0);
  $('localOpen').setAttribute(     "defaultAction", gOpenMode == 1);
  $('remoteDownload').setAttribute("defaultAction", gOpenMode == 0);
  $('remoteOpen').setAttribute(    "defaultAction", gOpenMode == 1);

  $('localUpload').setAttribute(   "key", gOpenMode == 0 ? "key_transfer" : "");
  $('localOpen').setAttribute(     "key", gOpenMode == 1 ? "key_transfer" : "key_open");
  $('remoteDownload').setAttribute("key", gOpenMode == 0 ? "key_transfer" : "");
  $('remoteOpen').setAttribute(    "key", gOpenMode == 1 ? "key_transfer" : "key_open");
}

function onLocalPathFocus(event) {
  gLocalPathFocus  = gLocalPath.value;
}

function onLocalPathBlur(event) {
  gLocalPath.value = gLocalPathFocus;
}

function onRemotePathFocus(event) {
  gRemotePathFocus = gRemotePath.value;
}

function onRemotePathBlur(event) {
    gRemotePath.value = gRemotePathFocus;
}

function browseLocal(title) {          // browse the local file structure
  var nsIFilePicker = Components.interfaces.nsIFilePicker;
  var fp            = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
  fp.init(window, title ? title : gStrbundle.getString("selectFolder"), nsIFilePicker.modeGetFolder);
  var res = fp.show();
	
  if (res == nsIFilePicker.returnOK) {
    localDirTree.changeDir(fp.file.path);
  }

  return res == nsIFilePicker.returnOK;
}

function browseLocal2(title) {          // browse the local file structure
  var nsIFilePicker = Components.interfaces.nsIFilePicker;
  var fp            = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
  fp.init(window, title ? title : gStrbundle.getString("selectFolder"), nsIFilePicker.modeGetFolder);
  var res = fp.show();
  
  if (res == nsIFilePicker.returnOK) {
	  
    localDirTree2.changeDir(fp.file.path);
  }

  return res == nsIFilePicker.returnOK;
}

function parseSize(size) {             // adds byte information for file sizes
  if (size >= 1024 * 1024 * 1024) {
    size = gStrbundle.getFormattedString("gigabyte", [parseFloat(size / 1024 / 1024 / 1024).toFixed(1).replace(/\./g, gStrbundle.getString("decimal"))]);
  } else if (size >= 1024 * 1024) {
    size = gStrbundle.getFormattedString("megabyte", [parseFloat(size / 1024 / 1024).toFixed(2).replace(/\./g, gStrbundle.getString("decimal"))]);
  } else if (size >= 1024) {
    size = gStrbundle.getFormattedString("kilobyte", [parseFloat(size / 1024).toFixed(1).replace(/\./g, gStrbundle.getString("decimal"))]);
  } else if (size >= 0) {
    size = gStrbundle.getFormattedString("bytes", [size]);
  }

  return size;
}

function displayWelcomeMessage(msg) {
  if (gWelcomeMode) {
    try {
      if (gWelcomeWindow && gWelcomeWindow.close) {          // get rid of those extra pestering welcome windows if the program is reconnecting automatically
        gWelcomeWindow.close();
      }
    } catch (ex) { }

    gWelcomeWindow = window.openDialog("chrome://fireftp/content/welcome.xul", "welcome", "chrome,resizable,centerscreen", msg);
  }
}

function custom() {
  if (!gFtp.isConnected || !isReady()) {
    return;
  }

  var cmd = window.prompt(gStrbundle.getString("command"), "", gStrbundle.getString("customCommand"));

  if (!cmd) {
    return;
  }

  gFtp.custom(cmd);
}

function cloneObject(what) {
  for (i in what) {
    this[i] = what[i];
  }
}

function cloneArray(a) {
  var n = new Array();

  for (var x = 0; x < a.length; ++x) {
    n.push(a[x]);
  }

  return n;
}

function runInFirefox(path) {
  var windowManager          = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService();
  var windowManagerInterface = windowManager.QueryInterface(Components.interfaces.nsIWindowMediator);
  var win                    = windowManagerInterface.getMostRecentWindow("navigator:browser");

  if (win) {
    var theTab               = win.gBrowser.addTab(path);
    win.gBrowser.selectedTab = theTab;
    return;
  }

  try {    // this is used if FireFTP is running as a standalone and there are no browsers open; much more complicated, not very pretty
    var firefoxInstallPath = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties)
                                       .get("CurProcD", Components.interfaces.nsILocalFile);
    var firefox            = localFile.init(firefoxInstallPath.path + "\\" + "firefox.exe");

    if (!firefox.exists()) {                                 // try linux
      firefox.initWithPath(firefoxInstallPath.path + "/" + "firefox");
      if (!firefox.exists()) {                               // try os x
         firefox.initWithPath(firefoxInstallPath.path + "/" + "firefox-bin");
      }
    }

    var process = Components.classes['@mozilla.org/process/util;1'].createInstance(Components.interfaces.nsIProcess);
    process.init(firefox);
    var arguments = new Array(path);
    process.run(false, arguments, arguments.length, {});
  } catch (ex) {
    debug(ex);
  }
}

function tipJar() {
  if (!gDonated) {
    gPrefs.setBoolPref("donated", true);
    runInFirefox("http://fireftp.mozdev.org/donate.html");
  }
}

function doAbort() {
  gSearchRunning = false;
  var forceKill  = false;

  if (gFxp && gFxp.isConnected) {
    gFxp.disconnect();
    forceKill = true;
  }

  for (var x = 0; x < gMaxCon; ++x) {
    gConnections[x].abort(forceKill);
    forceKill = false;
  }

  queueTree.failed = new Array();
}

function toolsPopupMenu() {
  $('diffMenuItem').setAttribute("disabled",     !gFtp.isConnected || localTree.searchMode == 2 || remoteTree.searchMode == 2);
  $('recDiffMenuItem').setAttribute("disabled",  !gFtp.isConnected || localTree.searchMode == 2 || remoteTree.searchMode == 2);
}

function setCharAt(str, index, ch) {                         // how annoying
  return str.substr(0, index) + ch + str.substr(index + 1);
}

// thanks to David Huynh
// http://mozilla-firefox-extension-dev.blogspot.com/2004/11/passing-objects-between-javascript.html
function wrapperClass(obj) {
  this.wrappedJSObject = this;
  this.obj             = obj;
}

wrapperClass.prototype = {
  QueryInterface : function(iid) {
    if (iid.equals(Components.interfaces.nsISupports)) {
      return this;
    }

    throw Components.results.NS_ERROR_NO_INTERFACE;
  }
}

function getPlatform() {
	var platform = navigator.platform.toLowerCase();
	
	if (platform.indexOf('linux') != -1) {
		return 'linux';
	}

	if (platform.indexOf('mac') != -1) {
		return 'mac';
	}

	return 'windows';
}

function doResizeReverseHack() {        // bah, humbug
  $('localdirtree').setAttribute(    'flex', '1');
  $('localdirtree').removeAttribute( 'width');
  $('remotedirtree').setAttribute(   'flex', '1');
  $('remotedirtree').removeAttribute('width');
  $('localview').setAttribute(       'flex', '1');
  $('localview').removeAttribute(    'width');
  $('remoteview').setAttribute(      'flex', '1');
  $('remoteview').removeAttribute(   'width');

  doResizeHack();
}

function doResizeHack() {
  $('localdirtree').setAttribute(    'width', $('localdirtree').treeBoxObject.width);
  $('localdirtree').removeAttribute( 'flex');
  $('remotedirtree').setAttribute(   'width', $('remotedirtree').treeBoxObject.width);
  $('remotedirtree').removeAttribute('flex');
  $('localview').setAttribute(       'width', $('localview').boxObject.width);
  $('localview').removeAttribute(    'flex');
  $('remoteview').setAttribute(      'width', $('remoteview').boxObject.width);
  $('remoteview').removeAttribute(   'flex');
}

function testAccelKey(event) {
  if (getPlatform() == 'mac') {
    return event.metaKey;
  }

  return event.ctrlKey;
}
