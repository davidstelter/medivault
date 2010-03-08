function encryptButton() {

	var count = {};
	var items = acos5.SPRS_listCerts(count)// getting the list of certs.

		//prompting with a list box with the certs available.
		var prompts = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);

	//var items = ["cert1", "cert2", "cert3", "cert4"];

	var selected = {};

	var result = prompts.select(null, "Select Cert", "Please choose a certificate", items.length,items, selected);

	// selected cert index
	var index = selected.value;

	// getting the output file name.
	var FileName = prompt("Please enter the file name: ");

	//Getting the input file name and path.
	var tree = document.getElementById("localtree");
	var cellIndex = 0;
	var cellText = tree.view.getCellText(tree.currentIndex, tree.columns.getColumnAt(cellIndex));

	var FileIn = document.getElementById('localpath').value + cellText; 

	//Getting the path for the output file.
	var FileOut = document.getElementById('remotepath').value + FileName;

	//alert("File Out\n" + FileOut + "\n File In!! \n" + FileIn + '\n' + "cert!! \n"+ items[index]);   

	encryptFile(FileIn, FileOut, items[index]);
}

//get file selected in local (left) file tree
function getLocalFileSelection(){
	var tree = document.getElementById("localtree");
	var cellIndex = 0;
	var cellText = tree.view.getCellText(tree.currentIndex, tree.columns.getColumnAt(cellIndex));

	return document.getElementById('localpath').value + cellText; 
}

//get file selected in remote (right) file tree
function getRemoteFileSelection(){
	var tree = document.getElementById("remotetree");
	var cellIndex = 0;
	var cellText = tree.view.getCellText(tree.currentIndex, tree.columns.getColumnAt(cellIndex));

	return document.getElementById('remotepath').value + cellText; 
}

function loadButton(){
	var FileIn = getRemoteFileSelection();
	var clear = loadFile(FileIn);
	if(clear!=null){
		myWindow = window.open('','','resizable=yes,scrollbars=yes,width=1000,height=800');
		myWindow.document.body.innerHTML = "<div><pre>" + clear +"</pre></div>";
	}

}

function cardSelectDialog(){

	//Getting the slot  
	var prompts = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);

	//var items = ["cert1", "cert2", "cert3", "cert4"];
	var items = acos5.SPRS_enumerateCards({});

	var selected = {};

	var result = prompts.select(null, "Select Card", "Please choose a card", items.length,items, selected);

	var index = selected.value;

//	alert(index);

	var attempt=1;
	do{
		var pin = prompt("Please enter your PIN");
		var login = acos5.SPRS_selectCard(index, pin);
		if(login)
			appendLog("Login successful");
		else{
			appendLog("Login attempt "+attempt+" failed!");
		}
		++attempt;
	}
	while(!login && attempt < 3);

	if(!login){
		alert("Login failed after " + attempt +" attempts!");
		finalizeCrypto();
	}
}
