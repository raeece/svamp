%{
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

        extern "C" int yyparse (void);
        extern int yylex();
	extern FILE* yyin;
        extern FILE* outnewickfile;
        extern FILE* parserrorfile;
        extern "C" char *yytext;    /* Correct for Flex */
        extern "C" int yycolumn;    /* Correct for Flex */

        extern void* saferealloc(void *ptr, size_t size);
	int yywrap() { return 1;}
        //void yyerror(const char* s) {fprintf(stderr,"Error:at postition %d  near \"%s\".%s\n",(yycolumn-1),yytext,s);}
	//void yyerror(const char* s) { fprintf(stderr," %s: '%s' in line %d\n", msg, yytext);}
        void yyerror(const char *s);

	struct tree_t
	{
		char* label;
		char* length;
		struct tree_t* child;
		struct tree_t* next;
	};

	typedef struct tree_t Tree;

	static Tree* newTree()
	{
		Tree* t=saferealloc(0,sizeof(Tree));
		memset(t,0,sizeof(Tree));
		return t;
	}
	static void freeTree(Tree* t)
	{
		if(t==NULL) return;
		free(t->label);
		free(t->length);
		freeTree(t->child);
		freeTree(t->next);
		free(t);
	}

	static void escape(FILE* out,const char* s)
	{

		if(s==NULL) { fputs("null",out); return;}
		fputc('\"',out);
		while(*s!=0)
		{
			switch(*s)
			{
				case '\'': fputs("\\\'",out); break;
				case '\"': fputs("\\\"",out); break;
				case '\\': fputs("\\\\",out); break;
				case '\n': fputs("\\n",out); break;
				case '\r': fputs("\\r",out); break;
				case '\t': fputs("\\t",out); break;
				default : fputc(*s,out); break;
			}
			++s;
		}
		fputc('\"',out);
	}
	static void num(FILE* out,const char* d)
	{
		if(d==NULL) { fputs("null",out);; return;}
		fputs(d,out);
	}

	static void printTree(FILE* out,const Tree* t)
	{
		int comma=0;
		fputs("{",out);
		if(t->label!=NULL) { fputs("\"label\":",out);escape(out,t->label); comma++;}
		if(t->length!=NULL) {if(comma!=0) fputc(',',out);fputs("\"length\":",out);num(out,t->length);comma++;}


		if(t->child!=0)
		{
			const Tree* c=t->child;
			int i=0;
			if(comma!=0) {fputc(',',out); ++comma;}
			fputs("\"children\":[",out);

			while(c!=NULL)
			{
				if(i!=0) fputc(',',out);
				printTree(out,c);
				c=c->next;
				++i;
			}
			fputc(']',out);
		}
		fputs("}",out);
	}

	static void printprettyTree(FILE* out,const Tree* t,int tab)
	{
		int i=0;
		for(i=0;i<tab;i++){
			fputs("\t",out);
		}
		if(t->label!=NULL) { fputs("\"label\":",out);escape(out,t->label);}
		if(t->length!=NULL) {fputs("\"length\":",out);num(out,t->length);}
		fputs("\n",out);	
		if(t->child!=0)
		{
			i++;
			const Tree* c=t->child;
			while(c!=NULL)
			{
				printprettyTree(out,c,i);
				c=c->next;
			}
		}
	}
	static void printprexmlTree(FILE* out,const Tree* t)
	{

			if(t->child!=0){
				fputs("begin_internal_",out);
			}
			else{
				fputs("begin_leaf_",out);
			}
		escape(out,t->label);
		fputs("_",out);
		num(out,t->length);
		fputs("\n",out);
		if(t->child!=0)
		{
			const Tree* c=t->child;
			while(c!=NULL)
			{
				printprexmlTree(out,c);
				c=c->next;
			}
		}
		if(t->child!=0){
			fputs("end_internal_",out);
		}
		else{
			fputs("end_leaf_",out);
		}
		escape(out,t->label);
		fputs("\n",out);
	}

        static void printnormalisedTree(FILE* out,const Tree* t)
        {

                int comma=0;
                if(t->child!=0){
                //  internal node
                fputs("(",out);
                }
                else{
                // leaf node
                fputs(t->label,out); comma++;
                if(t->length!=NULL) {fputc(':',out);num(out,t->length);comma++;}
                else{ fputc(':',out);num(out,"0.0");comma++;}
                }

                if(t->child!=0)
                {
                        const Tree* c=t->child;
                        int i=0;
                        if(comma!=0) {fputc(',',out); ++comma;}
                        //fputs("(",out);

                        while(c!=NULL)
                        {
                                if(i!=0) fputc(',',out);
                                printnormalisedTree(out,c);
                                c=c->next;
                                ++i;
                        }
                        //fputc(')',out);
                }
                // end of internal node
                if(t->child!=0){
                    fputs(")",out);
                    if(t->label!=NULL){
                    fputs(t->label,out);
                    }
                    fputc(':',out);
                    if(t->length!=NULL){
                    fputs(t->length,out);
                    }
                    else{
                    fputs("0.0",out);
                    }
                }



         }
	%}


	%union  {
		char* s;
		char* d;
		struct tree_t* tree;
	}

%error-verbose
%debug 
%locations
%token OPAR
%token CPAR
%token COMMA
%token COLON SEMICOLON 
%token<s> STRING
%token<d> NUMBER
%type<s> label optional_label
%type<d> number optional_length
%type<tree> subtree descendant_list_item descendant_list
%start input
%%

input: descendant_list optional_label optional_length SEMICOLON
{
	Tree* tree=newTree();
	//tree->type=ROOT;
	tree->child=$1;
	tree->label=$2;
	tree->length=$3;
	//printTree(stdout,tree);
        //printprexmlTree(stdout,tree);
        printnormalisedTree(outnewickfile,tree);
	freeTree(tree);
};

descendant_list: OPAR  descendant_list_item CPAR
{
	$$=$2;
};

descendant_list_item: subtree
{
	$$=$1;
}
|descendant_list_item COMMA subtree
{
	Tree* last=$1;
	$$=$1;
	while(last->next!=NULL)
	{
		last=last->next;
	}
	last->next=$3;
}
;

subtree : descendant_list optional_label optional_length
{
	$$=newTree();
	$$->child=$1;
	$$->label=$2;
	$$->length=$3;
}
| label optional_length
{
	$$=newTree();
	$$->label=$1;
	$$->length=$2;
}
;

optional_label:  { $$=NULL;} | label  { $$=$1;};
optional_length:  { $$=NULL;} | COLON number { $$=$2;};
label: NUMBER {$$=$1} | STRING { $$=$1;};
number: NUMBER { $$=$1;};



%%

/*int main(int argc,char** argv)
{
	yyin=stdin;
	yyparse();
	return EXIT_SUCCESS;
}
*/
void yyerror(const char *s)
{
   // qDebug() << "Error at position "<<(yycolumn-1)<<"near "<<yytext<<" "<< s;
   fprintf(parserrorfile,"Position:%d\t",(yycolumn-1));
   fprintf(parserrorfile,"near:%s\terrormsg:%s \n",yytext,s);
}
