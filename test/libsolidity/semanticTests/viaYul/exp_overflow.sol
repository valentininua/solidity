contract C {
    function f(uint8 x, uint8 y) public returns (uint) {
        return x**y;
    }
    function g(uint x, uint y) public returns (uint) {
        return x**y;
    }
}
// ====
// compileViaYul: true
// ----
// f(uint8,uint8): 2, 7 -> 0x80
// f(uint8,uint8): 2, 8 -> FAILURE
// f(uint8,uint8): 15, 2 -> 225
// f(uint8,uint8): 6, 3 -> 0xd8
// f(uint8,uint8): 7, 2 -> 0x31
// f(uint8,uint8): 7, 3 -> FAILURE
// f(uint8,uint8): 7, 4 -> FAILURE
// g(uint256,uint256): 0x200000000000000000000000000000000, 1 -> 0x0200000000000000000000000000000000
// g(uint256,uint256): 0x100000000000000000000000000000010, 2 -> FAILURE
// g(uint256,uint256): 0x200000000000000000000000000000000, 2 -> FAILURE
// g(uint256,uint256): 0x200000000000000000000000000000000, 3 -> FAILURE
