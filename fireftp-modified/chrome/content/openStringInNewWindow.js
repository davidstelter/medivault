function openStringInNewWindow() {			

/*
// getting the selected file name.
   var tree = document.getElementById("localtree"); 
   var cellIndex = 0;
   var cellText = tree.view.getCellText(tree.currentIndex, tree.columns.getColumnAt(cellIndex));
   
//combine the selected file name with the path
   var FileIn = document.getElementById('localpath').value + '\\' + cellText; 
   
//calling the load file function and passing the whole path.
 var  aString = acos5.SPRS_loadFile( FileIn)

*/

var aString = 'majed love pizza';// temp for testing.

//Displaying the content of the file in a new browser window.
myWindow = window.open('','','resizable=yes,scrollbars=yes,width=1000,height=800');
myWindow.document.body.innerHTML = "<div>" + aString +"</div>";

}
