// One Line Comment

/* Multi
 * Line
 * Comment
 */

//var a = 4;
//var b = 2;
//var result = a + b;

function my_function(a)
{
    //var result;
//
    //result = 20;

    return (a + 5.31) * 0.2345;
}

var a = 0;
var b = 256;
var c;
var result = 2;

var stuckRef = globalThis;

label loop;

var max_iteration = 1000000;

for (var index = 0; index <= max_iteration; index+=max_iteration/100)
{
    globalThis[index] = my_function(index);

    globalThis["test_plugin_function"](globalThis[index]);
}

globalThis.something_that.do_not_exists = 0;

//if (result)
//{
//    test_plugin_function(((a % b) + 1) == b, a, b);
//}
//else
//{
//    goto end_thing;
//}
