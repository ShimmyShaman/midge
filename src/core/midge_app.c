/* midge_app.c */

#include "core_definitions.h"
#include "render/render_thread.h"
#include <time.h>

#include "core/midge_app.h"

// void *callit(void *state)
// {
//   printf("!!callit-mc %p\n", state);
//   render_thread_info *global_data->render_thread = (render_thread_info *)state;

//   global_data->render_thread->thread_info->has_concluded = 1;
//   return NULL;
// }

// int dothecall(void *something)
// {
//   void *(*rout)(void *) = (void *(*)(void *))something;
//   printf("dothecall\n");
//   rout(NULL);
//   printf("dothecall-after-rout-call\n");
//   return 0;
// }

// int begin_silly_thread()
// {
//   printf("begin_silly_thread\n");
//   dothecall(&callit);
//   // dothecall(callit); // TODO -- C implicity converts the function so its the same as the above line, this doesn't
//   // work atm in midge...

//   return 0;
// }

int begin_render_thread()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->render_thread = (render_thread_info *)malloc(sizeof(render_thread_info));
  printf("global_data->render_thread = %p\n", global_data->render_thread);
  global_data->render_thread->render_thread_initialized = false;
  {
    // Resource Queue
    pthread_mutex_init(&global_data->render_thread->resource_queue.mutex, NULL);
    global_data->render_thread->resource_queue.count = 0;
    global_data->render_thread->resource_queue.allocated = 0;

    // Render Queue
    pthread_mutex_init(&global_data->render_thread->render_queue.mutex, NULL);
    global_data->render_thread->render_queue.count = 0;
    global_data->render_thread->render_queue.allocated = 0;

    pthread_mutex_init(&global_data->render_thread->input_buffer.mutex, NULL);
    global_data->render_thread->input_buffer.event_count = 0;
  }

  // -- Start Thread
  // dothecall(&callit);
  // printf("global_data->render_thread:%p &global_data->render_thread:%p\n", global_data->render_thread,
  // global_data->render_thread);

  begin_mthread(&midge_render_thread, &global_data->render_thread->thread_info, (void *)global_data->render_thread);
}

void complete_midge_app_compile()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  const char *remainder_app_source_files[] = {
      // "src/ui/ui_definitions.h",
      // "src/control/mc_controller.h",
      "src/modules/app_modules.h",

      "src/render/render_common.c",
      "src/env/hierarchy.c",
      "src/env/util.c",
      "src/env/project_management.c",
      "src/env/global_context_menu.c",
      "src/env/global_root.c",
      "src/ui/controls/panel.c",
      "src/ui/controls/text_block.c",
      "src/ui/controls/button.c",
      "src/ui/controls/context_menu.c",
      "src/ui/ui_functionality.c",
      // "src/ui/ui_render.c",
      "src/control/mc_controller.c",

      // Modules
      "src/modules/modus_operandi/modus_operandi_curator.c",
      "src/modules/hierarchy_viewer/hierarchy_viewer.c",
      "src/modules/source_editor/source_editor_pool.c",
      "src/modules/source_editor/function_editor.c",
      "src/modules/source_editor/source_line.c",
      NULL,
  };

  for (int f = 0; remainder_app_source_files[f]; ++f) {
    printf("instantiate file:'%s'\n", remainder_app_source_files[f]);
    instantiate_all_definitions_from_file(global_data->global_node, remainder_app_source_files[f], NULL);
  }
}

extern "C" {
void mcc_initialize_input_state();
void mcc_update_xcb_input();
void mui_initialize_core_ui_components();
void mui_render_element_headless(mc_node *element_node);
void mui_render_element_present(image_render_queue *render_queue, mc_node *element_node);

// Modules
// void init_modus_operandi_curator();
}

void initialize_midge_components()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mcc_initialize_input_state();

  mui_initialize_ui_state(&global_data->ui_state);
  mui_initialize_core_ui_components();

  // Environment
  mca_init_global_context_menu();
  mca_init_global_node_context_menu_options();
  mca_init_source_editor_pool();
  // mca_init_visual_project_management();

  // Modules
  // init_modus_operandi_curator();
  init_hierarchy_viewer();
}

void midge_initialize_app(struct timespec *app_begin_time)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Set Times
  global_data->app_begin_time = app_begin_time;

  struct timespec source_load_complete_time;
  clock_gettime(CLOCK_REALTIME, &source_load_complete_time);

  printf("#######################\n  <<<< MIDGE >>>>\n\nCore Compile took %.2f "
         "seconds\n",
         source_load_complete_time.tv_sec - global_data->app_begin_time->tv_sec +
             1e-9 * (source_load_complete_time.tv_nsec - global_data->app_begin_time->tv_nsec));

  // Asynchronously begin the render thread containing vulkan and xcb
  begin_render_thread();

  // Compile the remainder of the application
  complete_midge_app_compile();
  printf("midge compilation complete\n");

  // Complete Initialization of the global root node
  mca_init_node_layout(&global_data->global_node->layout);
  global_data->global_node->layout->__requires_rerender = true;

  // Initialize main thread
  initialize_midge_components();
  printf("midge components initialized\n");

  // Wait for render thread initialization and all initial resources to load before
  // continuing with the next set of commands
  bool waited = false;
  int count = 0;
  while (!global_data->render_thread->render_thread_initialized || global_data->render_thread->resource_queue.count) {
    waited = !global_data->render_thread->render_thread_initialized;
    usleep(1);
    ++count;
    if (count % 100000 == 0) {
      printf("global_data->render_thread = %p %i %u\n", global_data->render_thread,
             global_data->render_thread->render_thread_initialized, global_data->render_thread->resource_queue.count);
    }
  }

  // Set properties

  // Completed
  struct timespec load_complete_frametime;
  clock_gettime(CLOCK_REALTIME, &load_complete_frametime);
  printf("##################################\n"
         "         <<<< MIDGE >>>>\n"
         "\n"
         "Post-Core load took %.2f seconds (*%s)\n"
         "App Begin took %.2f seconds\n"
         "##################################\n",
         load_complete_frametime.tv_sec - source_load_complete_time.tv_sec +
             1e-9 * (load_complete_frametime.tv_nsec - source_load_complete_time.tv_nsec),
         waited ? "vulkan-initialization limited" : "midge-compile-load limited",
         load_complete_frametime.tv_sec - global_data->app_begin_time->tv_sec +
             1e-9 * (load_complete_frametime.tv_nsec - global_data->app_begin_time->tv_nsec));
}

void mca_render_presentation()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  image_render_queue *sequence;
  obtain_image_render_queue(&global_data->render_thread->render_queue, &sequence);
  sequence->render_target = NODE_RENDER_TARGET_PRESENT;
  sequence->clear_color = COLOR_MIDNIGHT_EXPRESS;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  sequence->image_width = global_data->screen.width;
  sequence->image_height = global_data->screen.height;
  sequence->data.target_image.image_uid = global_data->present_image_resource_uid;

  for (int a = 0; a < global_data->global_node->children->count; ++a) {
    mc_node *child = global_data->global_node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_presentation)(image_render_queue *, mc_node *) =
          (void (*)(image_render_queue *, mc_node *))child->layout->render_present;
      render_node_presentation(sequence, child);
    }
  }
}

void midge_run_app()
{
  printf("~midge_run_app()~\n");

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
  mc_node *global_root_node = global_data->global_node;
  // printf("defaultfont-0:%u\n", global_data->default_font_resource);
  // printf("global_data->ui_state:%p\n", global_data->ui_state);

  struct timespec prev_frametime, current_frametime, logic_update_frametime;
  clock_gettime(CLOCK_REALTIME, &current_frametime);
  clock_gettime(CLOCK_REALTIME, &logic_update_frametime);

  int DEBUG_secs_of_last_5sec_update = 0;

  frame_time *elapsed = (frame_time *)calloc(sizeof(frame_time), 1);
  int ui = 0;
  while (1) {
    if (global_data->render_thread->thread_info->has_concluded) {
      printf("Render Thread Aborted abnormally.\nShutting down...\n");
      break;
    }

    // // Time
    bool logic_update_due = false;
    {
      // TODO DEBUG
      usleep(10);

      long ms;  // Milliseconds
      time_t s; // Seconds
      memcpy(&prev_frametime, &current_frametime, sizeof(struct timespec));
      clock_gettime(CLOCK_REALTIME, &current_frametime);

      elapsed->frame_secs = current_frametime.tv_sec - prev_frametime.tv_sec;
      elapsed->frame_nsecs = current_frametime.tv_nsec - prev_frametime.tv_nsec;
      if (elapsed->frame_nsecs < 0) {
        --elapsed->frame_secs;
        elapsed->frame_nsecs += 1e9;
      }
      elapsed->app_secs = current_frametime.tv_sec - global_data->app_begin_time->tv_sec;
      elapsed->app_nsecs = current_frametime.tv_nsec - global_data->app_begin_time->tv_nsec;
      if (elapsed->app_nsecs < 0) {
        --elapsed->app_secs;
        elapsed->app_nsecs += 1e9;
      }

      // Logic Update
      const int ONE_MS_IN_NS = 1000000;
      const int FIFTY_PER_SEC = 20 * ONE_MS_IN_NS;
      if (1e9 * (current_frametime.tv_sec - logic_update_frametime.tv_sec) + current_frametime.tv_nsec -
              logic_update_frametime.tv_nsec >
          FIFTY_PER_SEC) {
        logic_update_due = true;

        logic_update_frametime.tv_nsec += FIFTY_PER_SEC;
        if (logic_update_frametime.tv_nsec > 1000 * ONE_MS_IN_NS) {
          logic_update_frametime.tv_nsec -= 1000 * ONE_MS_IN_NS;
          ++logic_update_frametime.tv_sec;
        }
      }

      // Update Timers
      bool exit_gracefully = false;
      // for (int i = 0; i < !exit_gracefully &&
      // global_data->update_timers.count; ++i) {
      //   update_callback_timer *timer =
      //   global_data->update_timers.callbacks[i];

      //   if (!timer->update_delegate || !(*timer->update_delegate)) {
      //     continue;
      //   }

      //   // if (logic_update_due) {
      //   //   printf("%p::%ld<>%ld\n", timer->update_delegate,
      //   timer->next_update.tv_sec, current_frametime.tv_sec);
      //   // }
      //   if (current_frametime.tv_sec > timer->next_update.tv_sec ||
      //       (current_frametime.tv_sec == timer->next_update.tv_sec &&
      //        current_frametime.tv_nsec >= timer->next_update.tv_nsec)) {
      //     // Update
      //     {
      //       void *vargs[2];
      //       vargs[0] = (void *)&elapsed;
      //       vargs[1] = (void *)&timer->state;
      //       int mc_res = (*timer->update_delegate)(2, vargs);
      //       if (mc_res) {
      //         printf("--timer->update_delegate(2, vargs):%i\n", mc_res);
      //         printf("Ending execution...\n");
      //         exit_gracefully = true;
      //         break;
      //       }
      //     }

      //     if (timer->reset_timer_on_update)
      //       increment_time_spec(&current_frametime, &timer->period,
      //       &timer->next_update);
      //     else
      //       increment_time_spec(&timer->next_update, &timer->period,
      //       &timer->next_update);
      //   }
      // }
      if (exit_gracefully) {
        break;
      }

      // Special update
      // if (current_frametime.tv_sec - DEBUG_secs_of_last_5sec_update > 4) {
      //   DEBUG_secs_of_last_5sec_update = current_frametime.tv_sec;
      //   if (special_update) {
      //     void *vargs[1];
      //     vargs[0] = &elapsed;
      //     MCcall(special_update(1, vargs));
      //   }
      // }
    }

    // Handle Input
    pthread_mutex_lock(&global_data->render_thread->input_buffer.mutex);

    if (global_data->input_state_requires_update || global_data->render_thread->input_buffer.event_count > 0)
      mcc_update_xcb_input();

    // printf("~main_input\n");
    pthread_mutex_unlock(&global_data->render_thread->input_buffer.mutex);

    if (global_data->exit_requested)
      break;
    if (global_data->render_thread->thread_info->has_concluded) {
      printf("RENDER-THREAD closed unexpectedly! Shutting down...\n");
      break;
    }

    // Update State
    // TODO ?? -- update registered methods...

    // Update Visible Layout
    {
      // As is global node update despite any requirement
      // mca_update_node_list_logic(global_data->global_node->children);
      if (global_root_node->layout->__requires_layout_update) {
        for (int a = 0; a < global_data->global_node->children->count; ++a) {
          mc_node *child = global_data->global_node->children->items[a];
          if (child->layout && child->layout->determine_layout_extents) {
            // TODO fptr casting
            void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
                (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
            determine_layout_extents(child, LAYOUT_RESTRAINT_NONE);
          }
        }

        // Update the layout
        global_root_node->layout->__bounds = {0.f, 0.f, (float)global_data->screen.width,
                                              (float)global_data->screen.height};
        for (int a = 0; a < global_root_node->children->count; ++a) {
          mc_node *child = global_root_node->children->items[a];

          if (child->layout && child->layout->update_layout) {
            // TODO fptr casting
            void (*update_node_layout)(mc_node *, mc_rectf *) =
                (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
            update_node_layout(child, &global_root_node->layout->__bounds);
          }
        }
        global_root_node->layout->__requires_layout_update = false;
        printf("layout updated\n");
      }

      // TODO Get rid of this field maybe?
      // global_data->ui_state->requires_update = false;
    }

    if (global_data->exit_requested)
      break;

    // Render State Changes
    if (global_root_node->layout->__requires_rerender) {
      pthread_mutex_lock(&global_data->render_thread->render_queue.mutex);

      // Clear the render queue
      global_data->render_thread->render_queue.count = 0;

      // Rerender headless images
      // printf("headless\n");
      for (int a = 0; a < global_root_node->children->count; ++a) {
        mc_node *child = global_root_node->children->items[a];
        if (child->layout && child->layout->visible && child->layout->render_headless) {
          // TODO fptr casting
          // printf("rh-child-type:%i\n", child->type);
          void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
          render_node_headless(child);
        }
      }

      // Queue the updated render
      // printf("present\n");
      mca_render_presentation();

      // Release lock and allow rendering
      pthread_mutex_unlock(&global_data->render_thread->render_queue.mutex);

      // Reset States
      global_root_node->layout->__requires_rerender = false;
    }
  }
}

void midge_cleanup_app()
{
  printf("~midge_cleanup_app()~\n");

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // TODO invoke release resources on children...

  // End render thread
  end_mthread(global_data->render_thread->thread_info);

  // Destroy render thread resources
  pthread_mutex_destroy(&global_data->render_thread->resource_queue.mutex);
  pthread_mutex_destroy(&global_data->render_thread->render_queue.mutex);
  pthread_mutex_destroy(&global_data->render_thread->input_buffer.mutex);

  free(global_data->render_thread);
}