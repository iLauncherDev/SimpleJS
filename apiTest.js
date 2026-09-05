// One Line Comment

/* Multi
 * Line
 * Comment
 */

//var a = 4;
//var b = 2;
//var result = a + b;

//class my_class {
//    var default_value = 0;
//
//    function new()
//    {
//        this.default_value = 20;
//    }
//}
//
//var my_instance = new my_class();

function indirect_return(a)
{
    return a;
}

function my_function(a)
{
    //var result;
//
    //result = 20;

    return (indirect_return(a) + 5.31) * 0.2345;
}

var a = 0;
var b = 256;
var c;
var result = 2;

var stuckRef = globalThis;

label loop;

var test_plugin_function = globalThis["test_plugin_function"];

var max_iteration = 1000000;

for (var index = 0; index <= max_iteration; index+=max_iteration/100)
{
    globalThis[index] = my_function(index);

    test_plugin_function(globalThis[index]);
}

globalThis[0.123456789123456789] = 0;
globalThis[0.123456789] = 0;
globalThis[0.5555] = 0;
globalThis[0.5] = 0;
globalThis[0.25] = 0;
globalThis[0.125] = 0;
globalThis[0] = 0;

globalThis.something_that.do_not_exists = 0;

//if (result)
//{
//    test_plugin_function(((a % b) + 1) == b, a, b);
//}
//else
//{
//    goto end_thing;
//}
