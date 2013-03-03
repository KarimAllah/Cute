#ifndef __ARCH_VM_H
#define __ARCH_VM_H

#define PIN_AS_L3_TABLE(page)
#define PIN_AS_L2_TABLE(page)
#define PIN_AS_L1_TABLE(page)

#define SET_PML4E(_pml4e, _user_supervisor, _present, _read_write, _pml3_base) \
{	\
	(_pml4e)->user_supervisor = (_user_supervisor);	\
	(_pml4e)->present = (_present);	\
	(_pml4e)->read_write = (_read_write);	\
	(_pml4e)->pml3_base = (_pml3_base);	\
}

#define SET_PML3E(_pml3e, _user_supervisor, _present, _read_write, _pml2_base) \
{	\
	(_pml3e)->present = (_present);	\
	(_pml3e)->read_write = (_read_write);	\
	(_pml3e)->user_supervisor = (_user_supervisor);	\
	(_pml3e)->pml2_base = (_pml2_base); \
}

#define SET_PML2E(_pml2e, _user_supervisor, _present, _read_write, _pt_base) \
{	\
	(_pml2e)->present = (_present);	\
	(_pml2e)->read_write = (_read_write);	\
	(_pml2e)->user_supervisor = (_user_supervisor);	\
	(_pml2e)->pt_base = (_pt_base);	\
}

#define SET_PML1E(_pml1e, _user_supervisor, _present, _read_write, _page_base) \
{	\
	(_pml1e)->present = (_present);	\
	(_pml1e)->read_write = (_read_write);	\
	(_pml1e)->user_supervisor = (_user_supervisor);	\
	(_pml1e)->page_base = (_page_base);	\
}

#endif
