function encryptButton() {
	if(!checkForSession())
		return;

	var count = {};
	var items = acos5.SPRS_listCerts(count)// getting the list of certs.

	var selected = {};
	var result = gPromptService.select(null, "Select Cert", "Please choose a certificate", items.length,items, selected);
	//if(!result)
//		return;

	// selected cert index
	var index = selected.value;

	// getting the output file name.
	var FileName = {};
	result = gPromptService.prompt(null, "Enter Filename", "Please enter the output file name:", FileName, null, {});
//	if(!result)
//		return;

	var FileIn = getLocalFileSelection();

	//Getting the path for the output file.
	var FileOut = getRemoteDirSelection() + FileName.value;

	encryptFile(FileIn, FileOut, items[index]);
	//refresh file view, there's a new file
	localTree.refresh(false, true);
	localTree2.refresh(false, true);
}

function signButton() {
	if(!checkForSession())
		return;

	var count = {};
	var items = acos5.SPRS_listCerts(count)// getting the list of certs.


	var selected = {};
	var result = gPromptService.select(null, "Select Cert", "Please choose a certificate", items.length,items, selected);
	if(!result)
		return;

	// selected cert index
	var index = selected.value;

	// getting the output file name.
	var FileName = {};
	var ok = gPromptService.prompt(null, "Enter filename", "Please enter the output file name:", FileName,null,{});
	if(!ok)
		return;

	var FileOut = getRemoteDirSelection() + FileName.value;

	var FileIn = getLocalFileSelection();

	signFile(FileIn, FileOut, items[index]);
	//refresh file view, there's a new file
	localTree.refresh(false, true);
	localTree2.refresh(false, true);
}

function ensureSlash(path){
	if(path.charAt(path.length-1)!='\\')
		return path + '\\';
	else
		return path;
}

//get file selected in local (left) file tree
function getLocalFileSelection(){
	var tree = document.getElementById("localtree");
	var cellIndex = 0;
	var cellText = tree.view.getCellText(tree.currentIndex, tree.columns.getColumnAt(cellIndex));

	return getLocalDirSelection() + cellText; 
}

//get file selected in remote (right) file tree
function getRemoteFileSelection(){
	var tree = document.getElementById("remotetree");
	var cellIndex = 0;
	var cellText = tree.view.getCellText(tree.currentIndex, tree.columns.getColumnAt(cellIndex));

	return getRemoteDirSelection() + cellText; 
}

//get directory selected in local (left) file tree
function getLocalDirSelection(){
	return ensureSlash(document.getElementById('localpath').value);
}

//get directory selected in remote (right) file tree
function getRemoteDirSelection(){
	return ensureSlash(document.getElementById('remotepath').value);
}

function loadButton(){
	if(!checkForSession())
		return;
	var FileIn = getRemoteFileSelection();
	var clear = loadFile(FileIn);
	if(clear!=null){
		myWindow = window.open('','','resizable=yes,scrollbars=yes,width=1000,height=800');
		myWindow.document.body.innerHTML = "<div><pre>" + clear +"</pre></div>";
	}
}

function cardSelectDialog(){

	var items = acos5.SPRS_enumerateCards({});

	var selected = {};
	var result = gPromptService.select(null, "Select Card", "Please choose a card", items.length,items, selected);

	var index = selected.value;

	var attempt=0;
	var ok;
	do{
		var pin = {};
		ok = gPromptService.promptPassword(null, "Enter PIN", "Please enter your PIN", pin, null, {});
		if(ok){
		  var login = acos5.SPRS_selectCard(index, pin.value);
		  if(login)
			appendLog("Login successful");
		  else{
			appendLog("Login attempt "+attempt+" failed!");
		  }
		  ++attempt;
		}
		else
			break;
	}
	while(!login && attempt < 3);

	if(!ok){
		appendLog("Login aborted by user");
		finalizeCrypto();
		return;
	}

	if(!login){
		alert("Login failed after " + attempt +" attempts!");
		finalizeCrypto();
	}
}
