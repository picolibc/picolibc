/* extern (library) versions of inline functions defined in winnt.h */

void* GetCurrentFiber(void)
{
    void* ret;
    __asm__ volatile (
	      "movl	%%fs:0x10,%0"
	      : "=r" (ret) /* allow use of reg eax,ebx,ecx,edx,esi,edi */
		  :	
		);
    return ret;
}

void* GetFiberData(void)
{
    void* ret;
    __asm__ volatile (            
	      "movl	%%fs:0x10,%0\n"
	      "movl	(%0),%0"
	      : "=r" (ret) /* allow use of reg eax,ebx,ecx,edx,esi,edi */
		  :	
	      );
    return ret;
}

