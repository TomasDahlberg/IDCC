/*
!+
!	CODETYPE.H
!-
*/

#ifndef CODETYPE_H_INCLUDED
#define CODETYPE_H_INCLUDED

#include <stdio.h>

/* enbart för att spara Next_Instruktion */
#define SaveAddress(i) (SaveStack[savestk_pt++] = (i))
#define PopSavedAddress (SaveStack[--savestk_pt])

/* enbart för aktuella parametrar */
#define PushTree(i) (PushStack[pushstk_pt++] = (i))
#define PopTree()   (PushStack[--pushstk_pt])
#define LookTree()   (PushStack[pushstk_pt - 1])

typedef struct _timerLink {
    char abs[80];
    char rel[80];
    int whatday;
    int timerNo;
    struct _timerLink *next;
} timerLink;

struct _base_list ;

typedef struct Id_S {char IDENT[100];
		     struct Id_S *Trailer;} *Sequence_Of_Id;



typedef enum {int_const, bool_const, int_, bool, int_vec,
              bool_vec, proc, int_func, bool_func, int_var,
              bool_var, int_vec_ref, bool_vec_ref, int_func_id,
 	      bool_func_id, class_id, class, method} Id_Type;

typedef enum {f_int, f_bool, f_int_var, f_bool_var, f_int_vec_ref,
	      f_bool_vec_ref} Formal_Info;

typedef struct Seq_F {Formal_Info f_info;
		      char IDENT[100];
		      struct Seq_F *Trailer;
	             } *Sequence_Of_Formal_Info;

typedef union {int NUMBER;
	       int BOOL_CONST_VAL;
	       int address_offset;
	       struct { int start_address;
			Sequence_Of_Formal_Info formal_sequence;
		      } proc_info;
	       char *func_ref; /* Om type == int_func_id || bool_func_id */
	       struct _base_list *inheritance;
	      } Id_Info;
		
typedef struct {char IDENT[100];
		Id_Type type;
	        Id_Info info;
	       } Ident_Info;

typedef struct Seq_I {Ident_Info i_info;
		      struct Seq_I *Trailer;
	             } *Sequence_Of_Ident_Info;

typedef struct Ref_T {Ident_Info *ref_id;
		      int block_offset;
	             } *Ref_Table;


/*
!+
!  Definition av opkoder
!  TRAPZ ny kod
!-
*/  


typedef enum {LIT = 0
/*
, NEG = 8192, ADD = 8193, SUB = 8194, MUL = 8195
		,DIV = 8196, ODD = 8197, EQU = 8198, NEQ = 8199, LES = 8200
		,GRT = 8201, NGR = 8202, NLE = 8203, JMP = 16384, JPZ = 24576
		,LOD = 32768, STO = 40960, REF = 49152, GET = 8204, PUT = 8205
		,MRK = 8206, CAL = 57344, OP_INT = 8207, RET = 8208, INP = 8209
		,OUT = 8210, AND = 8211, OR  = 8212, NOT = 8213, TRAPZ = 8214
		,NO_OP = 8220, ASSIGN = 4711 
*/
}	op_code;	

typedef struct
{
    Id_Type type;
    void *i, *b;
} Actual_Info;

typedef struct sequence
{
    Actual_Info node;
    struct sequence *next;
} Sequence_Of_Actual_Info;

typedef struct
{
    op_code operator;
    Ref_Table operand_ref;
    int operand_const;
    Sequence_Of_Actual_Info *actual_sequence;
} Int_Node;

typedef struct
{
    op_code operator;
    Ref_Table operand_ref;
    int operand_const;
    Sequence_Of_Actual_Info *actual_sequence;
} Bool_Node;

typedef struct integer_tree
{                            
    Int_Node node;
    struct integer_tree *left, *right;
} Int_Tree;
    
typedef struct boolean_tree
{
    Bool_Node node;
    void *left, *right;
} Bool_Tree;


/* Descriptor for arrays */

typedef struct
{
    int offset;
    Int_Tree *lb, *ub;
} Dyn_Descriptor;

typedef struct link_desc 
{
    Dyn_Descriptor node;
    struct link_desc *next;
} Linked_Dyn_Descriptor;


typedef struct _base_list 
{
    struct _base_list *next;
    Ref_Table baseClass;
} base_list ;


typedef unsigned int Instruction;

/* Prototyp definitioner för VVS.YACC*/

void Push();
char *Pop();
Sequence_Of_Formal_Info Form_Seq_alloc();
Sequence_Of_Ident_Info Ident_Seq_alloc();
Ref_Table Ref_alloc();
Sequence_Of_Id Id_alloc();
void clear();
char *append();
void Init_Table();
void Inc_Block_Level();
void Dec_Block_Level();
void Inc_Frame_Offset();
Ident_Info *Member();
Ident_Info *LookUp();
void Declare_Info();
Ref_Table LookUpIdent();

/* Prototyp definitioner för CODEREP.C */

void Init_Code();
void Emit_Instruction();
void Over_Load_Address();
Int_Tree *C_Int_Tree();
Bool_Tree *C_Bool_Tree();
Int_Node C_inode();
Bool_Node C_bnode();
void Dyn_Push();
Dyn_Descriptor Dyn_Pop();
void Code_Ref_Int_Variable();
void Code_Ref_Int_Push();
void Code_Store_Int_Variable();
void Code_Ref_Bool_Variable();
void Code_Ref_Bool_Push();
void Code_Store_Bool_Variable();
void Code_Int_Expr();
void Code_Bool_Expr();
void Gen_Parameter_Transmission();
void trans_error();

typedef struct
    {
    unsigned int kod : 16;
    unsigned int source : 12;
    unsigned int break_point : 4;
    } CV;

/* Linked_Dyn_Descriptor *Dyn_Array_Desc_Stack; */  /* Descriptor for arrays */

#endif
