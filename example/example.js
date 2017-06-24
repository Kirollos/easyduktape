/*
create event with callback function (must be initialised from c++ side with ->RegisterEvent()
*/
$time.ev.on('ts_fire', function (ret) {
    /*
    This function will get called when FireEvent is called from c++ side
    */
    print("$time.ts got called! Value: " + ret);
});
print("Executed: " + $time.ts()); // Predefined function in duktape core

function main() // main function that will be called in c++ side
{
    print("Main called! :D");
    if($os.windows.iswindows == true) // Check if variable iswindows is true
    {
        var ret = $os.windows.setcolour(7, 0xd /* 13 */); // call the function and get its value
        print("We set color (back: " + (ret >> 4) + ", fore: " + (ret & 0xF) + ")"); // extract the ret value into back and fore with bitwise ops
    }
}