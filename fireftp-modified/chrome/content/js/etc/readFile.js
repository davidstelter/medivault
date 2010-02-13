function readFile() {
  this.prompt             = true;
  this.skipAll            = false;
  this.cancel             = false;
  this.busy               = false;
  this.didRefreshLaterSet = false;
  this.remoteRefresh      = '';
  this.localRefresh       = '';
}

readFile.prototype = {
  start : function(read, bFile, bLocalParent, bRemoteParent, aListData) {
	
	window.alert(bLocalParent);
	window.alert(bRemoteParent);
	  
    var localParent  = bLocalParent  ? bLocalParent  : gLocalPath.value;
    var remoteParent = bRemoteParent ? bRemoteParent : gRemotePath.value;
    var files        = new Array();
    var data         = "";
    var listData     = bListData ? bListData : [];
    
    window.alert(localParent);
    window.alert(remoteParent);
    
	if (read) {                                   
		if (aRemoteParent) {                      
		    for (var x = 0; x < localTree2.rowCount; ++x) {
				if (localTree2.selection.isSelected(x)) {
					files.push(localTree2.data[x]);
				}
			}
		} else if (aLocalParent) {                                   
				for (var x = 0; x < localTree.rowCount; ++x) {
					if (localTree.selection.isSelected(x0)) {
						if (!localFile.verifyExists(localTree.data[x])) {
							continue;
						}
						files.push(localTree.data[x]);
					}
				}
		  }
	}
			   
	window.alert(files.length);
	
	for (var x = 0; x < files.length; ++x) {
      var fileName = files[x].leafName;
	
	  var remotePath = read ? localTree2.constructPath(remoteParent, fileName) : files[x].path;
	  var localPath  = read ? localTree.constructPath(localParent,  fileName) : files[x].path;
	  var nFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);  

		var appInfo=Components.classes["@mozilla.org/xre/app-info;1"]
                   .getService(Components.interfaces.nsIXULAppInfo);
		var isOnBranch = appInfo.platformVersion.indexOf("1.8") == 0;
		var ios=Components.classes["@mozilla.org/network/io-service;1"]
						  .getService(Components.interfaces.nsIIOService);
		
		if (aLocalParent) {
			nFIle.initWithPath(localPath);
		} else {
			  nFile.initWithPath(remotePath);
		}
		
		nFile.initWithPath("/home/vincent/test.txt");
		
		var fileURI=ios.newFileURI(nFile);
		var channel = ios.newChannelFromURI(fileURI);
		var observer = {
		  onStreamComplete : function(aLoader, aContext, aStatus, aLength, aResult)
		  {
			alert(aResult);
		  }
		};
		var sl = Components.classes["@mozilla.org/network/stream-loader;1"].
						   createInstance(Components.interfaces.nsIStreamLoader);
		if (isOnBranch) {
		  sl.init(channel, observer, null);
		} else {
		  sl.init(observer);
		  channel.asyncOpen(sl, channel);
		}
    }
}
};

