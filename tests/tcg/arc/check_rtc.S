	.include "macros.inc"

	start
	test_name RTC0
	lr	r0,[timer_build]
	and.f	r0,r0,0x400
	beq	@.lfail
	sr	1,[0x103]
	lr	r2,[0x104]
	print	"......"
	lr	r0,[0x104]
	breq	r0,r2,@.lfail
	print	"Success\n"
	end

.lfail:
	print "Fail\n"
	end
