/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include <string.h>

/* counter for variable memory locations */
static int location = 0;
static char* scope = " ";
int main_already_seem = 0;
static void declarationError(TreeNode * t, char * message);

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case AssignK: break;
        case IfK: break;
        case WhileK: break;
        case ReturnK: break;
        case ParamsK: break;
        
        case CallK:
          if (st_lookup(t->attr.name, scope) == -1)
              declarationError(t, "function called was not declared");
          break;

        case FuncK:
          scope = t->attr.name;
          if(strcmp(t->attr.name, "main") == 0)
              main_already_seem = 1;

          if(st_lookup(t->attr.name, " ") == -1)
          {
              if(t->type==Integer)
                  st_insert(t->attr.name, t->lineno, location++, " ", "function", "int");
              else
                  st_insert(t->attr.name, t->lineno, location++, " ", "function", "void");
          }
          else 
          {
            declarationError(t, "name already used by another function");
          }
        default: break;
      }
      break;

  case ExpK:
      switch (t->kind.exp)
      { 
        case VectorK: break;
        case OpK: break;
        case ConstK: break;

        case IdK:
          if (st_lookup(t->attr.name, scope) == -1)
              declarationError(t, "variable not declared in this scope");
          else
            if(t->already_seem == 0)
                st_insert(t->attr.name, t->lineno, 0, scope, " ", " ");
          break;

        case TypeK:
          if(t->type == Integer) 
          {
              if(st_lookup(t->child[0]->attr.name, scope) == -1)
              {
                  st_insert(t->child[0]->attr.name, t->child[0]->lineno, location++, scope, "var", "int");
                  t->child[0]->already_seem = 1;
              } else 
              {
                  declarationError(t, "name already used by another variable previosly declared in this scope or by another function");
              }
          }
          else 
          {
              if(t->type == Void)
                  declarationError(t, "variable must not be declared as void");
          }          
          break;
        default: break;
      }
      break;

  default: break;
  }
}

static void mainError()
{ if (main_already_seem == 0) {fprintf(listing,"Error: main must be declared\n");
   Error = TRUE;}
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  { 
    mainError();
    fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void declarationError(TreeNode * t, char * message)
{ fprintf(listing,"Declaration error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");

          if ((t->attr.op == EQUAL) || (t->attr.op == LT) || (t->attr.op == GT) || (t->attr.op == DIFF) || (t->attr.op == LTEQ) 
            || (t->attr.op == GTEQ) )
            t->type = Boolean;
          else
            t->type = Integer;
          break;

        case ConstK: 
            t->type = Integer; 
            break;

        case IdK:
          t->type = Integer;
          break;

        case VectorK:
            t->type = Integer; break;

        case TypeK: break;

        default:
          break;
      }
      break;

    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;

        case AssignK:
          if (t->child[0]->type != t->child[1]->type)
            typeError(t->child[0],"assignment error: different types");
          break;

        case WhileK:
          if (t->child[0]->type == Integer)
            typeError(t->child[1],"while test is not Boolean");
          break;

        case CallK:
           if (checkFunctionType(t->attr.name)==1)
              t->type = Integer;
          else
              t->type = Void;
          break;

        case FuncK: break;

        default:
          break;
      }
      break;
      
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}