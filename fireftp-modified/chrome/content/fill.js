function fill() {

//call numerate function fo get an array of strings 

var slots = acos5.SPRS_enumerateCards();


//var slots = new Array("slot1","slot2","slot3");

//Filling the dropdown list with the cards available.
var list = document.getElementById('slots');

for(var i=0; i<slots.length; i++){

list.appendItem(slots[i], i);

}



}



function submit() {

var list = document.getElementById('slots');
//int cardNum = list.selectedIndex; -----> will be used when connected to the wrapper
//alert("Error!! \n"+ (list.selectedIndex) + '.'); ------>for testing


var pin = prompt("Please enter your pin number for "+ list.selectedItem.label + " :");
//alert("pin entered: \n"+ (pin) + '.');-----------------> for testing 

//sending the selection index and the pin to the wraper 
/*

var flag = acos5.SPRS_selectCard(cardNum, pin)


*/

window.close();
}
