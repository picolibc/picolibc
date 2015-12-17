/*

Copyright (c) 2014  Red Hat, Inc. All rights reserved.

This copyrighted material is made available to anyone wishing to use, modify,
copy, or redistribute it subject to the terms and conditions of the BSD
License.   This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
are incorporated in the source code or documentation are not subject to the BSD
License and may only be used or replicated with the express permission of
Red Hat, Inc.

*/
#include "memmodel.h"

	.data

	.global	__CIOBUF__
__CIOBUF__:
	.global	_CIOBUF_
_CIOBUF_:
	.space	75

	.text

	.global	_libgloss_cio_hook
_libgloss_cio_hook:
	.global C$$IO$$
C$$IO$$:
	nop
        ret_
