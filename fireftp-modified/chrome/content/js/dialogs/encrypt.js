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

	var FileIn = document.getElementById('localpath').value + /*'\\' +*/ cellText; 

	//Getting the path for the output file.
	var FileOut = document.getElementById('remotepath').value + '\\' + FileName;

	//alert("File Out\n" + FileOut + "\n File In!! \n" + FileIn + '\n' + "cert!! \n"+ items[index]);   

	encryptFile(FileIn, FileOut, items[index]);

}

function cardSelectDialog(){

	//Getting the slot  
	var prompts = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);

	//var items = ["cert1", "cert2", "cert3", "cert4"];
	var items = acos5.SPRS_enumerateCards({});

	var selected = {};

	var result = prompts.select(null, "Select Card", "Please choose a card", items.length,items, selected);

	// sellected cert index
	var index = selected.value;

//	alert(index);

	//Getting the pin
	var pin = prompt("Please enter your PIN");
	// loading the slot
	var login = acos5.SPRS_selectCard(index, pin);
	//TODO: loop here to allow retries
	if(login)
		appendLog("Login successful");
	else{
		appendLog("Login failed!");
		finalizeCrypto();
	}
}
