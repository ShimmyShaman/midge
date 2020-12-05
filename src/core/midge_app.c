/* midge_app.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#include "m_threads.h"
// #include "core/core_definitions.h"
#include "core/app_modules.h"
#include "core/midge_app.h"
// #include "render/render_common.h"
#include "render/render_thread.h"
// #include "ui/ui_definitions.h"
#include "control/mc_controller.h"

#include "midge_app.h"

// void *callit(void *state)
// {
//   printf("!!callit-mc %p\n", state);
//   render_thread_info *app_info->render_thread = (render_thread_info *)state;

//   app_info->render_thread->thread_info->has_concluded = 1;
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
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  app_info->render_thread = (render_thread_info *)malloc(sizeof(render_thread_info));
  // printf("app_info->render_thread = %p\n", app_info->render_thread);
  app_info->render_thread->render_thread_initialized = false;
  {
    // Resource Queue
    app_info->render_thread->resource_queue = (resource_queue *)malloc(sizeof(resource_queue));
    MCcall(pthread_mutex_init(&app_info->render_thread->resource_queue->mutex, NULL));
    app_info->render_thread->resource_queue->count = 0;
    app_info->render_thread->resource_queue->allocated = 0;

    // Render Queue
    app_info->render_thread->image_queue = (image_render_list *)malloc(sizeof(image_render_list));
    MCcall(pthread_mutex_init(&app_info->render_thread->image_queue->mutex, NULL));
    app_info->render_thread->image_queue->count = 0;
    app_info->render_thread->image_queue->alloc = 0;

    // Render Request Object Pool
    app_info->render_thread->render_request_object_pool = (image_render_list *)malloc(sizeof(image_render_list));
    MCcall(pthread_mutex_init(&app_info->render_thread->render_request_object_pool->mutex, NULL));
    app_info->render_thread->render_request_object_pool->count = 0;
    app_info->render_thread->render_request_object_pool->alloc = 0;

    pthread_mutex_init(&app_info->render_thread->input_buffer.mutex, NULL);
    app_info->render_thread->input_buffer.event_count = 0;
  }

  // -- Start Thread
  // dothecall(&callit);
  // printf("app_info->render_thread:%p &app_info->render_thread:%p\n", app_info->render_thread,
  // app_info->render_thread);
  // printf("&midge_render_thread=%p\n", &midge_render_thread);

  return begin_mthread(&midge_render_thread, &app_info->render_thread->thread_info, (void *)app_info->render_thread);
}

// typedef struct

// void complete_midge_app_compile()
// {
//   mc_app_info *app_info;
//   obtain_midge_global_root(&app_info);

//   const char *remainder_app_source_files[] = {
//       // "src/ui/ui_definitions.h",
//       "src/control/mc_controller.h",
//       // "src/modules/app_modules.h",
//       // "src/render/resources/hash_table.h",
//       // "src/render/resources/obj_loader.h",

//       // "src/render/resources/hash_table.c",
//       // "src/render/resources/obj_loader.c",
//       "src/render/render_common.c",
//       "src/env/hierarchy.c",
//       "src/env/util.c",
//       // "src/env/project_management.c",
//       // "src/env/global_context_menu.c",
//       // "src/env/global_root.c",
//       "src/ui/controls/panel.c",
//       "src/ui/controls/text_block.c",
//       "src/ui/controls/button.c",
//       // "src/ui/controls/context_menu.c",
//       "src/ui/ui_functionality.c",
//       // // "src/ui/ui_render.c",
//       "src/control/mc_controller.c",

//       // // Modules
//       // "src/modules/modus_operandi/modus_operandi_curator.c",
//       // "src/modules/hierarchy_viewer/hierarchy_viewer.c",
//       // "src/modules/source_editor/source_editor_pool.c",
//       // "src/modules/source_editor/function_editor.c",
//       // "src/modules/source_editor/source_line.c",
//       // "src/modules/three_d/three_d.c",
//       NULL,
//   };

//   for (int f = 0; remainder_app_source_files[f]; ++f) {
//     // instantiate_all_definitions_from_file(app_info->global_node, remainder_app_source_files[f], NULL);
//     puts("TODO Load file");
//   }
// }

// // extern "C" {
// void mcc_initialize_input_state();
// void mcc_update_xcb_input();
// // void mcu_initialize_core_ui_components();
// // void mcu_render_element_headless(mc_node *element_node);
// // void mcu_render_element_present(image_render_details *image_render_queue, mc_node *element_node);
// void mca_load_modules();
// void mca_load_previously_open_projects();
// // }

int initialize_midge_state()
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  MCcall(mcc_initialize_input_state());

  mcu_initialize_ui_state(&app_info->ui_state);
  return 0;
}

void *mca_load_modules_then_project_async(void *state)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  int res;
  // Modules
  res = mca_load_modules();
  if (res) {
    printf("--"
           "mca_load_modules"
           " line:%i:ERR:%i\n",
           __LINE__ - 5, res);

    app_info->_exit_requested = true;

    return NULL;
  }

  // Projects
  res = mca_load_previously_open_projects();
  if (res) {
    printf("--"
           "mca_load_previously_open_projects"
           " line:%i:ERR:%i\n",
           __LINE__ - 5, res);

    app_info->_exit_requested = true;

    return NULL;
  }

  res = mca_fire_event(MC_APP_EVENT_INITIAL_MODULES_PROJECTS_LOADED, NULL);
  if (res) {
    printf("--"
           "mca_load_previously_open_projects"
           " line:%i:ERR:%i\n",
           __LINE__ - 5, res);

    app_info->_exit_requested = true;

    return NULL;
  }

  struct timespec load_complete_time;
  clock_gettime(CLOCK_REALTIME, &load_complete_time);
  printf("[[MIDGE-App] > Modules/Projects Compile complete after %.2f seconds]\n",
         load_complete_time.tv_sec - app_info->app_begin_time->tv_sec +
             1e-9 * (load_complete_time.tv_nsec - app_info->app_begin_time->tv_nsec));
  // puts("modules & open-projects loading complete");

  return NULL;
}

int midge_initialize_app(struct timespec *app_begin_time)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // Set Times
  app_info->app_begin_time = app_begin_time;

  struct timespec source_load_complete_time;
  clock_gettime(CLOCK_REALTIME, &source_load_complete_time);

  printf("[[MIDGE-CORE] > Core Compile took %.2f seconds]\n",
         source_load_complete_time.tv_sec - app_info->app_begin_time->tv_sec +
             1e-9 * (source_load_complete_time.tv_nsec - app_info->app_begin_time->tv_nsec));

  // Asynchronously begin the render thread containing vulkan and xcb
  MCcall(begin_render_thread());

  // Complete Initialization of the global root node
  MCcall(mca_init_node_layout(&app_info->global_node->layout));
  app_info->global_node->layout->__requires_rerender = true;

  // Initialize main thread
  MCcall(initialize_midge_state());
  printf("midge components initialized\n");

  // Begin the async load thread of modules -then- projects
  mthread_info *modules_load_thr_info;
  MCcall(begin_mthread(&mca_load_modules_then_project_async, &modules_load_thr_info, NULL));

  // Wait for render thread initialization and all initial resources to load before
  // continuing with the next set of commands
  bool waited = false;
  int count = 0;
  while (!app_info->render_thread->render_thread_initialized || app_info->render_thread->resource_queue->count) {
    waited = !app_info->render_thread->render_thread_initialized;
    usleep(1);
    ++count;
    if (count % 100000 == 0) {
      printf("app_info->render_thread = %p %i %u\n", app_info->render_thread,
             app_info->render_thread->render_thread_initialized, app_info->render_thread->resource_queue->count);
    }
  }

  if (app_info->_exit_requested) {
    if (!app_info->render_thread->thread_info->has_concluded) {
      end_mthread(app_info->render_thread->thread_info);
    }

    return 1;
  }

  // while (!modules_load_thr_info->has_concluded) {
  //   // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  //   usleep(10);
  // }

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
         load_complete_frametime.tv_sec - app_info->app_begin_time->tv_sec +
             1e-9 * (load_complete_frametime.tv_nsec - app_info->app_begin_time->tv_nsec));

  return 0;
}

int mca_render_presentation()
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  image_render_details *image_render;
  MCcall(mcr_obtain_image_render_request(app_info->render_thread, &image_render));
  image_render->render_target = NODE_RENDER_TARGET_PRESENT;
  image_render->clear_color = COLOR_MIDNIGHT_EXPRESS;
  // printf("app_info->screen : %u, %u\n", app_info->screen.width,
  // app_info->screen.height);
  image_render->image_width = app_info->screen.width;
  image_render->image_height = app_info->screen.height;
  image_render->data.target_image.image = (mcr_texture_image *)app_info->screen.present_image;

  for (int a = 0; a < app_info->global_node->children->count; ++a) {
    mc_node *child = app_info->global_node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // // DEBUG - TIME
      // struct timespec debug_start_time, debug_end_time;
      // clock_gettime(CLOCK_REALTIME, &debug_start_time);
      // // DEBUG - TIME

      // TODO fptr casting
      void (*render_node_presentation)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_presentation(image_render, child);

      // // DEBUG - TIME
      // clock_gettime(CLOCK_REALTIME, &debug_end_time);
      // printf("'%s'-Rerender took %.2fms\n", child->name == NULL ? "(null)" : child->name,
      //        1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
      //            1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
      // // DEBUG - TIME
    }
  }

  MCcall(mcr_submit_image_render_request(app_info->render_thread, image_render));

  return 0;
}

int midge_run_app()
{
  printf("midge_run_app()\n");

  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);
  mc_node *global_root_node = app_info->global_node;
  // printf("defaultfont-0:%u\n", app_info->default_font_resource);
  // printf("app_info->ui_state:%p\n", app_info->ui_state);

  struct timespec prev_frametime, current_frametime, logic_update_frametime;
  clock_gettime(CLOCK_REALTIME, &current_frametime);
  clock_gettime(CLOCK_REALTIME, &logic_update_frametime);

  struct timespec debug_start_time, debug_end_time;

  int DEBUG_secs_of_last_5sec_update = 0;

  app_info->elapsed = (frame_time *)calloc(sizeof(frame_time), 1);
  frame_time *elapsed = app_info->elapsed;
  int ui = 0;
  while (1) {
    if (app_info->render_thread->thread_info->has_concluded) {
      printf("Render Thread Aborted abnormally.\nShutting down...\n");
      break;
    }

    // // Time
    bool logic_update_due = false;
    {
      // TODO DEBUG
      // usleep(10000);

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
      elapsed->frame_secsf = (float)elapsed->frame_secs + 1e-9 * elapsed->frame_nsecs;

      elapsed->app_secs = current_frametime.tv_sec - app_info->app_begin_time->tv_sec;
      elapsed->app_nsecs = current_frametime.tv_nsec - app_info->app_begin_time->tv_nsec;
      if (elapsed->app_nsecs < 0) {
        --elapsed->app_secs;
        elapsed->app_nsecs += 1e9;
      }
      elapsed->app_secsf = (float)elapsed->app_secs + 1e-9 * elapsed->app_nsecs;

      // Logic Update
      const int ONE_MS_IN_NS = 1000000;
      const int FIFTY_PER_SEC = 20 * ONE_MS_IN_NS;
      if (1e9 * (current_frametime.tv_sec - logic_update_frametime.tv_sec) + current_frametime.tv_nsec -
              logic_update_frametime.tv_nsec >
          FIFTY_PER_SEC) {
        logic_update_due = true;
        // printf("global_root_node->layout->__requires_rerender = %i\n",
        // global_root_node->layout->__requires_rerender);

        logic_update_frametime.tv_nsec += FIFTY_PER_SEC;
        if (logic_update_frametime.tv_nsec > 1000 * ONE_MS_IN_NS) {
          logic_update_frametime.tv_nsec -= 1000 * ONE_MS_IN_NS;
          ++logic_update_frametime.tv_sec;
        }
      }

      // Update Timers
      bool exit_gracefully = false;
      // for (int i = 0; i < !exit_gracefully &&
      // app_info->update_timers.count; ++i) {
      //   update_callback_timer *timer =
      //   app_info->update_timers.callbacks[i];

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
    if (app_info->input_state_requires_update || app_info->render_thread->input_buffer.event_count) {
      clock_gettime(CLOCK_REALTIME, &debug_start_time);

      MCcall(pthread_mutex_lock(&app_info->render_thread->input_buffer.mutex));
      mcc_update_xcb_input();
      MCcall(pthread_mutex_unlock(&app_info->render_thread->input_buffer.mutex));

      clock_gettime(CLOCK_REALTIME, &debug_end_time);
      // printf("input_state_update took %.2f ms\n", 1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
      //                                                 1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
    }

    // printf("~main_input\n");

    if (app_info->_exit_requested)
      break;
    if (app_info->render_thread->thread_info->has_concluded) {
      printf("RENDER-THREAD closed unexpectedly! Shutting down...\n");
      break;
    }

    // Update State
    // TODO ?? -- update registered methods...

    // Update Visible Layout
    {
      // As is global node update despite any requirement
      // mca_update_node_list_logic(app_info->global_node->children);
      if (global_root_node->layout->__requires_layout_update) {
        clock_gettime(CLOCK_REALTIME, &debug_start_time);
        // printf("layout-locking... %p\n", &app_info->hierarchy_mutex);
        MCcall(pthread_mutex_lock(&app_info->hierarchy_mutex));
        // puts("layout-locked");
        global_root_node->layout->__requires_layout_update = false;

        for (int a = 0; a < app_info->global_node->children->count; ++a) {
          mc_node *child = app_info->global_node->children->items[a];
          if (child->layout && child->layout->determine_layout_extents) {
            // TODO fptr casting
            void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
                (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
            determine_layout_extents(child, LAYOUT_RESTRAINT_NONE);
          }
        }

        // Update the layout
        global_root_node->layout->__bounds =
            (mc_rectf){0.f, 0.f, (float)app_info->screen.width, (float)app_info->screen.height};
        for (int a = 0; a < global_root_node->children->count; ++a) {
          mc_node *child = global_root_node->children->items[a];

          if (child->layout && child->layout->update_layout) {
            // TODO fptr casting
            // printf("ulay-child-type:%i '%s'\n", child->type, child->name);
            void (*update_node_layout)(mc_node *, mc_rectf *) =
                (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
            update_node_layout(child, &global_root_node->layout->__bounds);
          }
        }

        MCcall(pthread_mutex_unlock(&app_info->hierarchy_mutex));
        // puts("layout-unlock");

        clock_gettime(CLOCK_REALTIME, &debug_end_time);
        printf("LayoutUpdate took %.2fms\n", 1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
                                                 1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
      }

      // TODO Get rid of this field maybe?
      // app_info->ui_state->requires_update = false;
    }

    if (app_info->_exit_requested)
      break;

    // Render State Changes
    // TODO -- do not know how to eloquently handle render update requests outpacing the render thread
    if (global_root_node->layout->__requires_rerender && !app_info->render_thread->image_queue->count) {
      clock_gettime(CLOCK_REALTIME, &debug_start_time);

      // printf("child-order:\n");
      // for (int a = 0; a < global_root_node->children->count; ++a) {
      //   mc_node *child = global_root_node->children->items[a];

      //   printf(":%i:%p\n", a, child);
      // }

      // Reset States
      // printf("rerender-locking... %p\n", &app_info->hierarchy_mutex);
      MCcall(pthread_mutex_lock(&app_info->hierarchy_mutex));
      // puts("rerender-locked");
      global_root_node->layout->__requires_rerender = false;

      // Rerender headless images
      // printf("headless\n");
      for (int a = 0; a < global_root_node->children->count; ++a) {
        mc_node *child = global_root_node->children->items[a];
        // printf("child-type:%i '%s'\n", child->type, child->name);
        if (child->layout && child->layout->visible && child->layout->render_headless &&
            child->layout->__requires_rerender) {
          // TODO fptr casting
          // printf("rh-child-type:%i '%s'\n", child->type, child->name);
          void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
          render_node_headless(child);
        }
      }

      // Queue the updated render
      // printf("present\n");
      mca_render_presentation();

      MCcall(pthread_mutex_unlock(&app_info->hierarchy_mutex));
      // puts("rerender-unlock");

      // Release lock and allow rendering
      clock_gettime(CLOCK_REALTIME, &debug_end_time);
      printf("Rerender took %.2fms\n", 1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
                                           1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
    }
  }

  free(elapsed);
  app_info->elapsed = NULL;

  return 0;
}

void midge_cleanup_app()
{
  printf("~midge_cleanup_app()~\n");

  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // TODO invoke release resources on children...

  // End render thread
  end_mthread(app_info->render_thread->thread_info);

  // Destroy render thread resources
  pthread_mutex_destroy(&app_info->render_thread->resource_queue->mutex);
  pthread_mutex_destroy(&app_info->render_thread->image_queue->mutex);
  pthread_mutex_destroy(&app_info->render_thread->input_buffer.mutex);

  free(app_info->render_thread);
  app_info->render_thread = NULL;

  // App Info
  mc_destroy_midge_app_info();
}