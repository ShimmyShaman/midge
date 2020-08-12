GOAL:
=======
Midges development goals:
-----------
  * Provide the capability to develop creative graphical applications via user input and commands.
    * To foster creation through quick reiterative processes.
  * Develop tools and interfaces that cultivate automation of user actions where beneficial.
    * Keep mind on collating sufficient context for the successful execution of automated commands.
    * To reduce dimensionality of the context and allowable outcomes in order to increase accuracy
        and reduce recovery from error.
    * Develop tools and allowances for as rich a user-input as possible. Starting with key-strokes
      and mouse-clicks with an eye on gestures and AV input.
    
###################################################################

Goal: Initiate and complete a project that can export an application that has a button that prints 'hello world' when pressed to console

    give it a window to start with
    function issues add button command to add the button
    issues handler attachment
    produces main() initialize_app() adds functionality


MAKE A SHITTY FIRST VERSION

work on interface a bit more so that I can develop the automation stuff more productively/usefully

make code-editor a moveable/resizeable window
make core-objects-display a moveable/resizeable window
do focus and Z-indexing

...data collection/analysis? interface?






exhibit the user configuration



make the whole init shebang - just through printf
 with the data inputs


looks for a sequence of characters



type 		s-p-e-c-i-a-l-_-d-a-t-a- -

label that	type-unit 'special_data'

pattern		[CODE_BLOCK/CLEAN]%{alpha}-%-%-%...
		&& matches_struct_name
		<DO>: init suggestion		
		

PROCESS WHEN ITS ALL SPIFFY
  origin obtains a action-database
  submits that with the action and context, obtains uid/TODO
  reports uid with abstracted effect {insert char 'c' etc}

  USER looks at the data collection
  LABELS data -- sees sequence 'special_data ' and labels it end result of that sequence
  LABELS data -- sees sequence '[special_data] %var ;' and labels as a PATTERN the end result of that sequence


Not only things to do, but things to keep track of


WHEN RECEIVE REPORT FROM 2424
IF 2424-state0->source_data->type == FUNCTION
AND
USE 2424 state0 as parameter to another method to


		





    Analyze syntax for patterns: 
        special_data s;
        s.num = 8;
        s.add = 3;

    local_declaration 



    best way forward is to start implementing in midge, then find ways to improve that process by watching what I do?








    IDE 2.0 - functions 2.0


























      // const char *commands =
      //     // Create invoke function script
      //     ".createScript\n"
      //     "nvi 'function_info *' finfo find_function_info nodespace @function_to_invoke\n"
      //     "ifs !finfo\n"
      //     "err 2455 \"Could not find function_info for specified function\"\n"
      //     "end\n"
      //     ""
      //     "dcs int rind 0\n"
      //     "dcl 'char *' responses[32]\n"
      //     ""
      //     "dcs int linit finfo->parameter_count\n"
      //     "ifs finfo->variable_parameter_begin_index >= 0\n"
      //     "ass linit finfo->variable_parameter_begin_index\n"
      //     "end\n"
      //     "for i 0 linit\n"
      //     "dcl char provocation[512]\n"
      //     "nvk strcpy provocation finfo->parameters[i]->name\n"
      //     "nvk strcat provocation \": \"\n"
      //     "$pi responses[rind] provocation\n"
      //     "ass rind + rind 1\n"
      //     "end for\n"
      //     // "nvk printf \"func_name:%s\\n\" finfo->name\n"
      //     "ifs finfo->variable_parameter_begin_index >= 0\n"
      //     "dcs int pind finfo->variable_parameter_begin_index\n"
      //     "whl 1\n"
      //     "dcl char provocation[512]\n"
      //     "nvk strcpy provocation finfo->parameters[pind]->name\n"
      //     "nvk strcat provocation \": \"\n"
      //     "$pi responses[rind] provocation\n"
      //     "nvi bool end_it strcmp responses[rind] \"finish\"\n"
      //     "ifs !end_it\n"
      //     "brk\n"
      //     "end\n"
      //     // "nvk printf \"responses[1]='%s'\\n\" responses[1]\n"
      //     "ass rind + rind 1\n"
      //     "ass pind + pind 1\n"
      //     "ass pind % pind finfo->parameter_count\n"
      //     "ifs pind < finfo->variable_parameter_begin_index\n"
      //     "ass pind finfo->variable_parameter_begin_index\n"
      //     "end\n"
      //     "end\n"
      //     "end\n"
      //     "$nv @function_to_invoke $ya rind responses\n"
      //     "|"
      //     "invoke_function_with_args_script|"
      //     "demo|"
      //     "invoke @function_to_invoke|"
      //     "mc_dummy_function|"
      //     ".runScript invoke_function_with_args_script|"
      //     "enddemo|"
      //     // // "demo|"
      //     // // "call dummy thrice|"
      //     // // "invoke mc_dummy_function|"
      //     // // "invoke mc_dummy_function|"
      //     // // "invoke mc_dummy_function|"
      //     // // "enddemo|"
      //     "demo|"
      //     "create function @create_function_name|"
      //     "construct_and_attach_child_node|"
      //     "invoke declare_function_pointer|"
      //     // ---- SCRIPT SEQUENCE ----
      //     // ---- void declare_function_pointer(char *function_name, char *return_type, [char *parameter_type,
      //     // ---- char *parameter_name]...);
      //     // > function_name:
      //     "@create_function_name|"
      //     // > return_type:
      //     "void|"
      //     // > parameter_type:
      //     "const char *|"
      //     // > parameter_name:
      //     "node_name|"
      //     // > Parameter 1 type:
      //     "finish|"
      //     // ---- END SCRIPT SEQUENCE ----
      //     // ---- SCRIPT SEQUENCE ----
      //     // ---- void instantiate_function(char *function_name, char *mc_script);
      //     "invoke instantiate_function|"
      //     "@create_function_name|"
      //     // "nvk printf \"got here, node_name=%s\\n\" node_name\n"
      //     "dcd node * child\n"
      //     "cpy char * child->name node_name\n"
      //     "ass child->parent command_hub->nodespace\n"
      //     "nvk append_to_collection (void ***)&child->parent->children &child->parent->children_alloc
      //     &child->parent->child_count
      //     "
      //     "(void *)child\n"
      //     "|"
      //     "enddemo|"
      //     // // -- END DEMO create function $create_function_name
      //     // "invoke force_render_update|"
      //     "invoke construct_and_attach_child_node|"
      //     "command_interface_node|"
      //     // "invoke set_nodespace|"
      //     // "command_interface_node|"

      //     // "create function print_word|"
      //     // "@create_function_name|"
      //     // "void|"
      //     // "char *|"
      //     // "word|"
      //     // "finish|"
      //     // "@create_function_name|"
      //     // "nvk printf \"\\n\\nThe %s is the Word!!!\\n\" word\n"
      //     // "|"
      //     // "invoke print_word|"
      //     // "===========$================$===============$============$============|"
      //     // clint->declare("void updateUI(mthread_info *p_render_thread) { int ms = 0; while(ms < 40000 &&"
      //     //                " !p_render_thread->has_concluded) { ++ms; usleep(1000); } }");

      // // Command Loop
      // printf("\n:> ");
      // int n = strlen(commands);
      // int s = 0;
      // char cstr[2048];
      // mc_process_action_v1 *suggestion = NULL;
      // void *vargs[12]; // TODO -- count
      // for (int i = 0; i < n; ++i) {
      //   if (commands[i] != '|')
      //     continue;
      //   strncpy(cstr, commands + s, i - s);
      //   cstr[i - s] = '\0';
      //   s = i + 1;

      //   vargs[0] = (void *)command_hub;
      //   vargs[4] = (void *)cstr;
      //   vargs[6] = (void *)&suggestion;

      //   if (!strcmp(cstr, "midgequit")) {
      //     printf("midgequit\n");
      //     break;
      //   }

      //   // printf("========================================\n");
      //   if (suggestion) {
      //     printf("%s]%s\n>: ", get_action_type_string(suggestion->type), suggestion->dialogue);
      //     release_process_action(suggestion);
      //     suggestion = NULL;
      //   }
      //   MCcall(submit_user_command(12, vargs));

      //   // if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_BROKEN)
      //   // {
      //   //   printf("\nUNHANDLED_COMMAND_SEQUENCE\n");
      //   //   break;
      //   // }
      //   // if (reply != NULL)
      //   // {
      //   //   printf("%s", reply);
      //   // }
      // }

create function with CI
edit that function



onscreen debugging of mouse position



visualNode
  - visualNode



globalRoot
  - visualNode::debugWindow
    - visualNode::textblock
	  - 
  - visualNode::commandinterface
    - visualNode::textbox
      - *ui_elements
        - background
        - textlines


control information
a visual to click on
ui system and focus setting
font & writing
submit to command system
bring up edit window
edit and save function




rendering system:
	is alerted to rerender a set of nodes and their children
	each node renders to its own image as a composite of itself and its children
  then rerenders all ascendants of the node
  
  need to know about the node:
    ptr to node
    delegate method which contains the primitive render instructions/or ptr to list containing
	    render calls...	
  
  
  
    ##################################################################################
    
    


.createscript finishThreshold = 1		 script = 1, demo = 2
	script_name:
	...
	resolved

create function construct_and_attach_child
unresolved_command

demo create function @{}
	demo_initiation
	demo invoke @{}
		demo_initiation
		.runscript
		function_name:
		@function_name
		...
		finish
		IDLE
		enddemo
		RESOLVED
	invoke initialize_function
		.runscript
		function_name:
		...
		RESOLVED
	...
	enddemo
	RESOLVED
invoke construct_and_attach_child
	.runscript
	...
	RESOLVED

	


Templates - confirmed processes
 - create function @{name_of_function_to_create}
    - declare function
    - define function
 - declare function
    - invoke declare_function
 - define function
    - invoke define_function
 - invoke @{name_of_function_to_invoke}
    ? sets contextual variable name_of_function_to_invoke
    - .runScript invoke_function_with_args



Process Matrix - previous history of actions for suggestions
	