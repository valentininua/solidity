pragma experimental SMTChecker;
contract C {
	bool a;
	bool b;
	bool c;
	bool d;
	bool e;
	bool f;
	function pressA() public {
		if(e) { a = true; } else { reset(); }
	}
	function pressB() public {
		if(c) { b = true; } else { reset(); }
	}
	function pressC() public {
		if(a) { c = true; } else { reset(); }
	}
	function pressD() public {
		d = true;
	}
	function pressE() public {
		if(d) { e = true; } else { reset(); }
	}
	function pressF() public {
		if(b) { f = true; } else { reset(); }
	}
	function is_not_solved() view public {
		assert(!f);
	}
	function reset() internal {
		a = false;
		b = false;
		c = false;
		d = false;
		e = false;
		f = false;
	}
}
// ----
// Warning 6328: (539-549): Assertion violation happens here
