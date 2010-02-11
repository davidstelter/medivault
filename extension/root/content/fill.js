function fill() {

//call numerate function fo get an array of strings "XCOPMP"

var slots = new Array("slot1","slot2","slot3");

var list = document.getElementById('slots');

for(var i=0; i<slots.length; i++){

list.appendItem(slots[i], i);

}



}



function submit() {

var list = document.getElementById('slots');

alert("Error!! \n"+ (list.selectedIndex) + '.');


var pin = prompt("Please enter your pin number for "+ list.selectedItem.label + " :");
alert("Error!! \n"+ (pin) + '.');

//send the selection index and the pin to the wraper "XCOPMP"

window.close();
}
