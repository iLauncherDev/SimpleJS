// One Line Comment

/* Multi
 * Line
 * Comment
 */

//var a = 4;
//var b = 2;
//var result = a + b;

function my_function(a, b)
{
    //var result;
//
    //result = 20;
}

var a = 0;
var b = 256;
var c;
var result = 2;

label loop;

if (result)
{
    test_plugin_function(((a % b) + 1) == b, a, b);
}
else
{
    goto end_thing;
}

a = a + 1;

goto loop;

label end_thing;

globTest = a;
