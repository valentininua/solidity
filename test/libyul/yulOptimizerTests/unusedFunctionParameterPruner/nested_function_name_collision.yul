// Test case where the name `g` occurs at two different places.
{
    function f(c) -> u
    {
        u := g(c)
        function g(d) -> w
        {
            w := 13
        }
    }

    function h(c) -> u
    {
        u := g(c)
        function g(d) -> w
        {
            w := 13
        }
    }

}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     function g() -> w
//     {
//         let w_2 := 13
//         w := w_2
//     }
//     function g_5(d) -> w
//     { w := g() }
//     function f(c) -> u
//     {
//         let u_1 := g_5(c)
//         u := u_1
//     }
//     function g_3() -> w_5
//     {
//         let w_5_4 := 13
//         w_5 := w_5_4
//     }
//     function g_3_6(d_4) -> w_5
//     { w_5 := g_3() }
//     function h(c_1) -> u_2
//     {
//         let u_2_3 := g_3_6(c_1)
//         u_2 := u_2_3
//     }
// }
