	.include "macros.inc"

	start

	test_name TRAP_1
	trap_s 0
	print "Pass\n"
	trap_s 1
	print "Pass\n"
	end

	.align 4
	.global EV_Trap
	.type EV_Trap, @function
EV_SWI:
EV_Trap:
	print "Pass\n"
	rtie
