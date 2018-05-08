
/* Register definitions */

/**
 * RVU VF LMT Line Registers
 */
union cavm_lmt_lf_lmtlinex {
	u64 u;
	struct lmt_lf_lmtlinex_s {
		u64 data;                           
	} s;
};

/**
 * RVU VF LMT Cancel Register
 */
union cavm_lmt_lf_lmtcancel {
	u64 u;
	struct lmt_lf_lmtcancel_s {
		u64 data;                           
	} s;
};
