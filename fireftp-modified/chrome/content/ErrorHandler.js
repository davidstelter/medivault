//This function takes two arguments, The error code with decides what do you want the error handler to ask the user for
// if you pass 1 the error handler will alert the user and do nothing
//if you pass 2 the error handler will ask the user to retry or cancel
//the second argument is the string you want to display for the user.
//Finally the function will return wether the user wants to retry as "1" or if he wants to cancel "0".
function ErrHand(ErrorCode, ErrorString) {

var ReturnVal = 0;

if (ErrorCode == 1) 
{
alert("Error!! \n"+ ErrorString + '.');
}
else
{
var temp =confirm('Error!! \n'+ ErrorString +'.');
if (temp==true)
  {
  ReturnVal = 1;
  }
else
  {
  ReturnVal = 0;
  }

}

return (ReturnVal);

}