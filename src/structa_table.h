#ifndef STRUCTA_TABLE_H_
#define STRUCTA_TABLE_H_

typedef void (*PFN_StructaGuiInit)(void);
typedef void (*PFN_StructaShutdownGui)(void);

typedef struct Structa_PFN_Table_T {
	PFN_StructaGuiInit StructaInitGui;
	PFN_StructaShutdownGui StructaShutdownGui;
} Structa_PFN_Table_T;
typedef struct Structa_PFN_Table_T* Structa_PFN_Table;

#endif // STRUCTA_TABLE_H_