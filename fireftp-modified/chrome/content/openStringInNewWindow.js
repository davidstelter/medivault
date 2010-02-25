function openStringInNewWindow() {			


//aString = acos5.SPRS_loadFile()

var aString = 'majed love pizza';

myWindow = window.open('','','resizable=yes,scrollbars=yes,width=1000,height=800');
myWindow.document.body.innerHTML = "<div>" + aString +"</div>";
	

}
