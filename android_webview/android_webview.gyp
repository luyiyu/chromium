# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'chromium_code': 1,
  },
  'includes': [
    'android_webview_tests.gypi',
  ],
  'targets': [
    {
      'target_name': 'libwebviewchromium',
      'type': 'shared_library',
      'android_unmangled_name': 1,
      'dependencies': [
        'android_webview_common',
      ],
      'ldflags': [
        # fix linking to hidden symbols and re-enable this (crbug.com/157326)
        '-Wl,--no-fatal-warnings'
      ],
      'sources': [
        'lib/main/webview_entry_point.cc',
      ],
      'conditions': [
        ['android_build_type != 0', {
          'libraries': [
            # The "android" gyp backend doesn't quite handle static libraries'
            # dependencies correctly; force this to be linked as a workaround.
            'cpufeatures.a',
          ],
        }],
      ],
    },
    {
      'target_name': 'android_webview_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_resources',
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_android_webview_pack',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_100_percent.pak',
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/android_webview_apk/assets/webviewchromium.pak',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
        }
      ],
    },
    {
      'target_name': 'android_webview_common',
      'type': 'static_library',
      'dependencies': [
        '../android_webview/native/webview_native.gyp:webview_native',
        '../components/components.gyp:auto_login_parser',
        '../components/components.gyp:navigation_interception',
        '../components/components.gyp:visitedlink_browser',
        '../components/components.gyp:visitedlink_renderer',
        '../components/components.gyp:web_contents_delegate_android',
        '../content/content.gyp:content',
        '../skia/skia.gyp:skia',
        '../ui/gl/gl.gyp:gl',
        'android_webview_pak',
      ],
      'include_dirs': [
        '..',
        '../skia/config',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/',
      ],
      'sources': [
        'browser/aw_browser_context.cc',
        'browser/aw_browser_context.h',
        'browser/aw_browser_main_parts.cc',
        'browser/aw_browser_main_parts.h',
        'browser/aw_contents_client_bridge_base.cc',
        'browser/aw_contents_client_bridge_base.h',
        'browser/aw_content_browser_client.cc',
        'browser/aw_content_browser_client.h',
        'browser/aw_contents_io_thread_client.h',
        'browser/aw_cookie_access_policy.cc',
        'browser/aw_cookie_access_policy.h',
        'browser/aw_devtools_delegate.cc',
        'browser/aw_devtools_delegate.h',
        'browser/aw_download_manager_delegate.cc',
        'browser/aw_download_manager_delegate.h',
        'browser/aw_http_auth_handler_base.cc',
        'browser/aw_http_auth_handler_base.h',
        'browser/aw_login_delegate.cc',
        'browser/aw_login_delegate.h',
        'browser/aw_quota_manager_bridge.cc',
        'browser/aw_quota_manager_bridge.h',
        'browser/aw_quota_permission_context.cc',
        'browser/aw_quota_permission_context.h',
        'browser/aw_request_interceptor.cc',
        'browser/aw_request_interceptor.h',
        'browser/aw_result_codes.h',
        'browser/browser_view_renderer.h',
        'browser/browser_view_renderer_impl.cc',
        'browser/browser_view_renderer_impl.h',
        'browser/find_helper.cc',
        'browser/find_helper.h',
        'browser/icon_helper.cc',
        'browser/icon_helper.h',
        'browser/input_stream.h',
        'browser/intercepted_request_data.h',
        'browser/jni_dependency_factory.h',
        'browser/net/android_stream_reader_url_request_job.cc',
        'browser/net/android_stream_reader_url_request_job.h',
        'browser/net/aw_network_delegate.cc',
        'browser/net/aw_network_delegate.h',
        'browser/net/aw_url_request_context_getter.cc',
        'browser/net/aw_url_request_context_getter.h',
        'browser/net/aw_url_request_job_factory.cc',
        'browser/net/aw_url_request_job_factory.h',
        'browser/net_disk_cache_remover.cc',
        'browser/net_disk_cache_remover.h',
        'browser/net/init_native_callback.h',
        'browser/net/input_stream_reader.cc',
        'browser/net/input_stream_reader.h',
        'browser/renderer_host/aw_render_view_host_ext.cc',
        'browser/renderer_host/aw_render_view_host_ext.h',
        'browser/renderer_host/aw_resource_dispatcher_host_delegate.cc',
        'browser/renderer_host/aw_resource_dispatcher_host_delegate.h',
        'browser/renderer_host/view_renderer_host.cc',
        'browser/renderer_host/view_renderer_host.h',
        'browser/scoped_allow_wait_for_legacy_web_view_api.h',
        'browser/scoped_allow_wait_for_legacy_web_view_api.h',
        'common/android_webview_message_generator.cc',
        'common/android_webview_message_generator.h',
        'common/aw_content_client.cc',
        'common/aw_content_client.h',
        'common/aw_hit_test_data.cc',
        'common/aw_hit_test_data.h',
        'common/aw_resource.h',
        'common/render_view_messages.cc',
        'common/render_view_messages.h',
        'common/renderer_picture_map.cc',
        'common/renderer_picture_map.h',
        'common/url_constants.cc',
        'common/url_constants.h',
        'lib/aw_browser_dependency_factory_impl.cc',
        'lib/aw_browser_dependency_factory_impl.h',
        'lib/main/aw_main_delegate.cc',
        'lib/main/aw_main_delegate.h',
        'public/browser/draw_gl.h',
        'renderer/aw_content_renderer_client.cc',
        'renderer/aw_content_renderer_client.h',
        'renderer/aw_render_process_observer.cc',
        'renderer/aw_render_process_observer.h',
        'renderer/aw_render_view_ext.cc',
        'renderer/aw_render_view_ext.h',
        'renderer/view_renderer.cc',
        'renderer/view_renderer.h',
      ],
    },
    {
      'target_name': 'android_webview_java',
      'type': 'none',
      'dependencies': [
        '../components/components.gyp:navigation_interception_java',
        '../components/components.gyp:web_contents_delegate_android_java',
        '../content/content.gyp:content_java',
        '../ui/ui.gyp:ui_java',
      ],
      'variables': {
        'java_in_dir': '../android_webview/java',
      },
      'includes': [ '../build/java.gypi' ],
    },
    {
      'target_name': 'android_webview_apk',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:base_java',
        '../components/components.gyp:navigation_interception_java',
        '../components/components.gyp:web_contents_delegate_android_java',
        '../content/content.gyp:content_java',
        '../media/media.gyp:media_java',
        '../net/net.gyp:net_java',
        '../ui/ui.gyp:ui_java',
        'libwebviewchromium',
      ],
      'variables': {
        'apk_name': 'AndroidWebView',
        'manifest_package_name': 'org.chromium.android_webview',
        'java_in_dir': '../android_webview/java',
        'native_libs_paths': ['<(SHARED_LIB_DIR)/libwebviewchromium.so'],
        'additional_input_paths': [
          '<(PRODUCT_DIR)/android_webview_apk/assets/webviewchromium.pak',
        ],
      },
      'includes': [ '../build/java_apk.gypi' ],
    },
    
    {
      'target_name': 'android_webview_package_resources',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:base_java',
        '../components/components.gyp:navigation_interception_java',
        '../components/components.gyp:web_contents_delegate_android_java',
        '../content/content.gyp:content_java',
        '../media/media.gyp:media_java',
        '../net/net.gyp:net_java',
        '../ui/ui.gyp:ui_java',
      ],

      'sources': [
        '>@(additional_R_files)',
        '>@(additional_R_text_files)',
        '>@(additional_res_dirs)',
      ],
      
      'actions': [
        {
          'action_name': 'copy_webview_package_resoruces',
          'inputs': [
            'build/copy_resources.py',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/android_webview_jar/res/ant-custom-rules.xml',
          ],
          'action': [
            'python',
            'build/copy_resources.py',
            '--additional-r-files',
            '>@(additional_R_files)',
            '--additional-r-text-files',
            '>@(additional_R_text_files)',
            '--generated-r-dirs',
            '>@(generated_R_dirs)',
            '--additional-res-packages',
            '>@(additional_res_packages)',
            '--additional-res-dirs',
            '>@(additional_res_dirs)',
            '--output-dir',
            '<(PRODUCT_DIR)/android_webview_jar/res/',
            '--output-ant-file',
            '<@(_outputs)',
          ]
        }
      ],
    },
    
    {
      'target_name': 'android_webview_package',
      'type': 'none',
      'dependencies': [
        'libwebviewchromium',
        'android_webview_java',
        'android_webview_package_resources',
      ],
      'variables': {
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/android_webview_jar/libs',
          'files': [
            '>@(input_jars_paths)',
          ]
        },
        {
          'destination': '<(PRODUCT_DIR)/android_webview_jar/assets',
          'files': [
            '<(PRODUCT_DIR)/android_webview_apk/assets/webviewchromium.pak',
          ]
        },
        {
          'destination': '<(PRODUCT_DIR)/android_webview_jar/libs/armeabi-v7a',
          'files': [
            '<(SHARED_LIB_DIR)/libwebviewchromium.so',
          ]
        },
        
      ],
    },
  ],
}
