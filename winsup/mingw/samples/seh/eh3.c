int
__except_handler3(
	struct _EXCEPTION_RECORD*	pExceptionRecord,
	struct EXCEPTION_REGISTRATION*	pRegistrationFrame,
	struct _CONTEXT*		pContextRecord,
	void*				pDispatcherContext
	)
{
	LONG	filterFuncRet;
	LONG trylevel;
	EXCEPTION_POINTERS exceptPtrs;
	PSCOPETABLE pScopeTable;


	CLD // Clear the direction flag (make no assumptions!)

	// if neither the EXCEPTION_UNWINDING nor EXCEPTION_EXIT_UNWIND bit
	// is set... This is true the first time through the handler (the
	// non-unwinding case)

	if ( ! (pExceptionRecord->ExceptionFlags
		 & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)
               ) )
	{
		// Build the EXCEPTION_POINTERS structure on the stack
		exceptPtrs.ExceptionRecord = pExceptionRecord;
		exceptPtrs.ContextRecord = pContextRecord;

		// Put the pointer to the EXCEPTION_POINTERS 4 bytes below the
		// establisher frame. See ASM code for GetExceptionInformation
		*(PDWORD)((PBYTE)pRegistrationFrame - 4) = &exceptPtrs;

		// Get initial "trylevel" value
		trylevel = pRegistrationFrame->trylevel 

		// Get a pointer to the scopetable array
		scopeTable = pRegistrationFrame->scopetable;

search_for_handler: 
		if ( pRegistrationFrame->trylevel != TRYLEVEL_NONE )
		{
			if ( pRegistrationFrame->scopetable[trylevel].lpfnFilter )
			{

				PUSH EBP // Save this frame EBP

			// !!!Very Important!!! Switch to original EBP. This is
			// what allows all locals in the frame to have the same
			// value as before the exception occurred.

				EBP = &pRegistrationFrame->_ebp

				// Call the filter function
				filterFuncRet = scopetable[trylevel].lpfnFilter();

				POP EBP // Restore handler frame EBP

				if ( filterFuncRet != EXCEPTION_CONTINUE_SEARCH )
				{
					if ( filterFuncRet < 0 ) // EXCEPTION_CONTINUE_EXECUTION
						return ExceptionContinueExecution;

					// If we get here, EXCEPTION_EXECUTE_HANDLER was specified
					scopetable == pRegistrationFrame->scopetable

			// Does the actual OS cleanup of registration frames
					// Causes this function to recurse
					__global_unwind2( pRegistrationFrame );


		// Once we get here, everything is all cleaned up, except
		// for the last frame, where we'll continue execution
					EBP = &pRegistrationFrame->_ebp

					__local_unwind2( pRegistrationFrame, trylevel );

		// NLG == "non-local-goto" (setjmp/longjmp stuff)
					__NLG_Notify( 1 ); // EAX == scopetable->lpfnHandler

		// Set the current trylevel to whatever SCOPETABLE entry
		// was being used when a handler was found
					pRegistrationFrame->trylevel = scopetable->previousTryLevel;

		// Call the _except {} block. Never returns.
					pRegistrationFrame->scopetable[trylevel].lpfnHandler();
				}
			}

			scopeTable = pRegistrationFrame->scopetable;
			trylevel = scopeTable->previousTryLevel

			goto search_for_handler;
		}
		else // trylevel == TRYLEVEL_NONE
		{
			retvalue == DISPOSITION_CONTINUE_SEARCH;
		}
	}
	else // EXCEPTION_UNWINDING or EXCEPTION_EXIT_UNWIND flags are set
	{
		PUSH EBP // Save EBP

		EBP = pRegistrationFrame->_ebp // Set EBP for __local_unwind2

		__local_unwind2( pRegistrationFrame, TRYLEVEL_NONE )

		POP EBP // Restore EBP

		retvalue == DISPOSITION_CONTINUE_SEARCH;
	}
}

