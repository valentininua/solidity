{
    function f(a) -> x
    {
        x := g(1)
        x := add(x, 1)
        function g(b) -> y
        {
            b := add(b, 1)
            y := mload(b)
        }
    }

    function h(c) -> u
    {
        u := j(c)
        function j(d) -> w
        {
            w := 13
            w := add(w, 1)
        }
    }

}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     function g(b) -> y
//     {
//         let b_8 := b
//         let _9 := 1
//         let b_3 := add(b_8, _9)
//         b := b_3
//         let y_4 := mload(b_3)
//         y := y_4
//     }
//     function f() -> x
//     {
//         let _10 := 1
//         let x_1 := g(_10)
//         x := x_1
//         let _11 := 1
//         let x_2 := add(x_1, _11)
//         x := x_2
//     }
//     function f_13(a) -> x
//     { x := f() }
//     function j() -> w
//     {
//         let w_6 := 13
//         w := w_6
//         let _12 := 1
//         let w_7 := add(w_6, _12)
//         w := w_7
//     }
//     function j_14(d) -> w
//     { w := j() }
//     function h(c) -> u
//     {
//         let u_5 := j_14(c)
//         u := u_5
//     }
// }
