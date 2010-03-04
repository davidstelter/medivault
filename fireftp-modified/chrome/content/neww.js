function temp() {

//var items = acos5.SPRS_listCerts()// getting the list of certs.

//prompting with a list box with the certs available.
var prompts = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);

var items = ["cert1", "cert2", "cert3", "cert4"];

var selected = {};

var result = prompts.select(null, "Select Cert", "Please choose a certificate", items.length,items, selected);

// sellected cert index
var index = selected.value;

// getting the output file name.
var FileName = prompt("Please enter the file name: ");




//***********************************************************************
//Getting the input file name and path.
   var tree = document.getElementById("localtree");
   var cellIndex = 0;
   var cellText = tree.view.getCellText(tree.currentIndex, tree.columns.getColumnAt(cellIndex));

   var FileIn = document.getElementById('localpath').value + '\\' + cellText; 

//**********************************************************************
//Getting the path for the output file.
   var FileOut = document.getElementById('remotepath').value + '\\' + FileName;
 
   alert("File Out\n" + FileOut + "\n File In!! \n" + FileIn + '\n' + "cert!! \n"+ index);   
//*********************************************************************

//acos5.SPRS_encryptFile(FileIn, FileOut, index)


}
