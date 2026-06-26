<?php

/**
 * @file
 * Block and chat UI helpers for AI Chatbot.
 */

/**
 * Returns asset module name for chatbot UI files.
 */
function ai_assistants_chatbot_asset_module() {
  return 'ai_chatbot';
}

/**
 * Shared block info builder.
 */
function ai_assistants_chatbot_block_info($delta, $label, $description) {
  return [
    $delta => [
      'info' => $label,
      'description' => $description,
      'cache' => BACKDROP_NO_CACHE,
    ],
  ];
}

/**
 * Shared block configure builder.
 */
function ai_assistants_chatbot_block_configure($delta = '', $settings = [], $target_delta = 'ai_assistants_chat_form') {
  $form = [];

  if ($delta === $target_delta) {
    $config = ai_assistants_block_normalize_settings($settings);
    $form['ai_assistants_block'] = [
      '#type' => 'container',
    ];
    $form['ai_assistants_block']['instance_id'] = [
      '#type' => 'hidden',
      '#value' => $config['instance_id'] !== '' ? $config['instance_id'] : ai_assistants_generate_uuid(),
    ];
    $form['ai_assistants_block']['custom_title'] = [
      '#type' => 'textfield',
      '#title' => t('Block title'),
      '#default_value' => $config['custom_title'],
      '#description' => t('Leave empty to hide the block title.'),
    ];
    $form['ai_assistants_block']['assistant_id'] = [
      '#type' => 'select',
      '#title' => t('Assistant'),
      '#options' => ai_assistants_options(),
      '#default_value' => $config['assistant_id'],
      '#required' => TRUE,
      '#description' => t('Select the Backdrop assistant that should power this block.'),
    ];
  }

  return $form;
}

/**
 * Shared block save handler.
 */
function ai_assistants_chatbot_block_save($delta = '', &$edit = [], $target_delta = 'ai_assistants_chat_form') {
  if ($delta !== $target_delta) {
    return;
  }

  $defaults = ai_assistants_block_default_config();
  $instance = !empty($edit['ai_assistants_block']) && is_array($edit['ai_assistants_block']) ? $edit['ai_assistants_block'] : [];
  $edit['ai_assistants_block'] = [
    'custom_title' => isset($instance['custom_title']) ? trim((string) $instance['custom_title']) : (!empty($defaults['custom_title']) ? (string) $defaults['custom_title'] : ''),
    'assistant_id' => isset($instance['assistant_id']) ? trim((string) $instance['assistant_id']) : (!empty($defaults['assistant_id']) ? (string) $defaults['assistant_id'] : ''),
    'instance_id' => !empty($instance['instance_id']) ? trim((string) $instance['instance_id']) : ai_assistants_generate_uuid(),
  ];
}

/**
 * Shared block view builder.
 */
function ai_assistants_chatbot_block_view($delta = '', $settings = [], $contexts = [], $target_delta = 'ai_assistants_chat_form') {
  if ($delta !== $target_delta) {
    return NULL;
  }

  $config = ai_assistants_block_normalize_settings($settings);
  $assistant_id = !empty($config['assistant_id']) ? (string) $config['assistant_id'] : '';
  if ($assistant_id === '' || empty(ai_assistants_load($assistant_id))) {
    return [
      'subject' => '',
      'content' => [
        '#markup' => '<p>' . t('Select an assistant in the block settings.') . '</p>',
      ],
    ];
  }

  $block_id = ai_assistants_block_instance_id($config);
  $form = backdrop_get_form('ai_assistants_chat_form', $block_id, $config);
  $title = !empty($config['custom_title']) ? check_plain($config['custom_title']) : NULL;
  $panel_id = 'ai-assistant-chatbot-panel-' . preg_replace('/[^a-z0-9_-]+/', '-', strtolower($block_id));

  $content = [];
  $content[] = [
    '#markup' => '<div class="chatbot-wrapper">'
      .   '<div class="chatbot-topbar">'
      .     (!empty($title) ? '<h2 class="block-title">' . $title . '</h2>' : '')
      .     '<button class="chat-toggle-button btn btn-accent1" type="button" aria-expanded="true" aria-controls="' . check_plain($panel_id) . '">💬 ' . t('Open Assistant') . '</button>'
      .   '</div>'
      .   '<div id="' . check_plain($panel_id) . '" class="chatbot-inner">',
  ];

  $content[] = $form;
  $content[] = ['#markup' => '</div></div>'
    . '<script>(function(){'
    . 'var p=' . json_encode($panel_id) . ';'
    . 'var k="ai-assistant-collapsed:"+p;'
    . 'var v=localStorage.getItem(k);'
    . 'if(v===null){v=localStorage.getItem("openai-assistant-collapsed");}'
    . 'if(v!=="true"){return;}'
    . 'var panel=document.getElementById(p);'
    . 'if(!panel){return;}'
    . 'panel.hidden=true;'
    . 'var sel=".block-ai-chatbot,.block-ai-assistant-chat,.block-ai-assistants-chat,.block-ai-chatbot-chat,.block-ai-chatbot-ai-chatbot-chat-form,.block-ai-assistants-ai-assistants-chat-form,.block-ai-assistant-ai-assistant-chat-form";'
    . 'var block=panel.closest(sel);'
    . 'if(block){block.classList.add("collapsed");}'
    . '})();</script>'];

  return [
    'subject' => '',
    'content' => $content,
  ];
}

/**
 * Shared block view alter helper.
 */
function ai_assistants_chatbot_block_view_alter(&$data, $block, $target_delta = 'ai_assistants_chat_form') {
  if ($block->delta === $target_delta) {
    $data['attributes']['class'][] = 'block-ai-chatbot';
    $data['attributes']['class'][] = 'block-ai-chatbot-chat';
    $data['attributes']['class'][] = 'block-ai-chatbot-' . str_replace('_', '-', $block->delta);
    $data['attributes']['class'][] = 'block-ai-assistants-chat';
    $data['attributes']['class'][] = 'block-ai-assistant-chat';
  }
}

/**
 * Formats plain assistant text into limited safe chat HTML.
 */
function ai_assistants_chat_format_text($text) {
  $text = trim((string) $text);
  if ($text === '') {
    return '';
  }
  $text = str_ireplace([
    '<br>',
    '<br/>',
    '<br />',
    '&lt;br&gt;',
    '&lt;br/&gt;',
    '&lt;br /&gt;',
    '&amp;lt;br&amp;gt;',
    '&amp;lt;br/&amp;gt;',
    '&amp;lt;br /&amp;gt;',
  ], "\n", $text);

  $lines = preg_split('/\r\n|\r|\n/', $text);
  $html = '';
  $paragraph = [];
  $list_type = '';

  $flush_paragraph = function () use (&$html, &$paragraph) {
    if (empty($paragraph)) {
      return;
    }
    $lines = array_map('ai_assistants_chat_format_inline', $paragraph);
    $html .= '<p>' . implode('<br>', $lines) . '</p>';
    $paragraph = [];
  };
  $close_list = function () use (&$html, &$list_type) {
    if ($list_type !== '') {
      $html .= '</' . $list_type . '>';
      $list_type = '';
    }
  };

  foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '') {
      $flush_paragraph();
      $close_list();
      continue;
    }

    if (preg_match('/^#{1,6}\s+(.+)$/', $line, $matches)) {
      $flush_paragraph();
      $close_list();
      $html .= '<p class="chat-history-heading">' . ai_assistants_chat_format_inline($matches[1]) . '</p>';
      continue;
    }

    if (preg_match('/^-{3,}$/', $line)) {
      $flush_paragraph();
      $close_list();
      $html .= '<hr>';
      continue;
    }

    if (preg_match('/^\d+\.\s+(.+)$/', $line, $matches)) {
      $flush_paragraph();
      if ($list_type !== 'ol') {
        $close_list();
        $html .= '<ol>';
        $list_type = 'ol';
      }
      $html .= '<li>' . ai_assistants_chat_format_inline($matches[1]) . '</li>';
      continue;
    }

    if (preg_match('/^[-*]\s+(.+)$/', $line, $matches)) {
      $flush_paragraph();
      if ($list_type !== 'ul') {
        $close_list();
        $html .= '<ul>';
        $list_type = 'ul';
      }
      $html .= '<li>' . ai_assistants_chat_format_inline($matches[1]) . '</li>';
      continue;
    }

    $close_list();
    $paragraph[] = $line;
  }

  $flush_paragraph();
  $close_list();

  return $html;
}

/**
 * Formats safe inline markdown-like markers after escaping text.
 */
function ai_assistants_chat_format_inline($text) {
  $text = check_plain((string) $text);
  $text = preg_replace_callback('/\[([^\]]+)\]\(([^)]+)\)/', function ($matches) {
    $label = $matches[1];
    $href = trim(html_entity_decode($matches[2], ENT_QUOTES | ENT_HTML5, 'UTF-8'));
    if ($href === '') {
      return $matches[0];
    }
    if (!preg_match('@^[a-z][a-z0-9+.-]*://@i', $href) && strpos($href, '//') !== 0 && $href[0] !== '/') {
      $href = '/' . $href;
    }
    return '<a href="' . check_url($href) . '">' . $label . '</a>';
  }, $text);
  $text = preg_replace('/`([^`]+)`/', '<code>$1</code>', $text);
  $text = preg_replace('/\*\*([^*]+)\*\*/', '<strong>$1</strong>', $text);
  return $text;
}

/**
 * Normalize a tool result payload that may be JSON or an array.
 */
function ai_assistants_normalize_tool_result($result) {
  if (is_array($result)) {
    return $result;
  }
  if (!is_string($result) || trim($result) === '') {
    return [];
  }

  $decoded = json_decode($result, TRUE);
  return is_array($decoded) ? $decoded : [];
}

/**
 * Extract RAG sources from an assistant/agent tool trace.
 */
function ai_assistants_extract_rag_sources(array $tool_calls) {
  $sources = [];
  foreach ($tool_calls as $entry) {
    if (empty($entry['tool']) || $entry['tool'] !== 'ai_search_rag_search') {
      continue;
    }

    $result = ai_assistants_normalize_tool_result(isset($entry['result']) ? $entry['result'] : NULL);
    if (empty($result['sources']) || !is_array($result['sources'])) {
      continue;
    }

    foreach ($result['sources'] as $source) {
      if (empty($source['url'])) {
        continue;
      }
      $url = function_exists('ai_search_normalize_relative_url')
        ? ai_search_normalize_relative_url($source['url'])
        : (strpos($source['url'], '/') === 0 ? $source['url'] : '/' . ltrim($source['url'], '/'));
      $key = !empty($source['source_id']) ? (string) $source['source_id'] : $url;
      $sources[$key] = [
        'title' => !empty($source['title']) ? (string) $source['title'] : $url,
        'url' => $url,
      ];
    }
  }

  return array_values($sources);
}

/**
 * Normalize an href/path for source matching.
 */
function ai_assistants_normalize_link_target($href) {
  $href = trim(html_entity_decode((string) $href, ENT_QUOTES | ENT_HTML5, 'UTF-8'));
  if ($href === '') {
    return '';
  }

  if (preg_match('@^[a-z][a-z0-9+.-]*://@i', $href) || strpos($href, '//') === 0) {
    $path = parse_url($href, PHP_URL_PATH);
    if (!empty($path)) {
      $href = $path;
    }
  }

  return strpos($href, '/') === 0 ? $href : '/' . ltrim($href, '/');
}

/**
 * Replace model-authored markdown links with links from real RAG sources only.
 */
function ai_assistants_sanitize_answer_links($answer, array $sources) {
  if (empty($sources)) {
    return (string) $answer;
  }

  $allowed = [];
  foreach ($sources as $source) {
    if (empty($source['url'])) {
      continue;
    }
    $canonical = ai_assistants_normalize_link_target($source['url']);
    if ($canonical === '') {
      continue;
    }
    $allowed[$canonical] = $source['url'];
  }

  return preg_replace_callback('/\[([^\]]+)\]\(([^)]+)\)/', function ($matches) use ($allowed) {
    $label = $matches[1];
    $normalized = ai_assistants_normalize_link_target($matches[2]);
    if ($normalized !== '' && !empty($allowed[$normalized])) {
      return '[' . $label . '](' . $allowed[$normalized] . ')';
    }
    return $label;
  }, (string) $answer);
}

/**
 * Append a canonical source list to one assistant answer.
 */
function ai_assistants_append_rag_sources($answer, array $sources) {
  if (empty($sources)) {
    return (string) $answer;
  }

  $lines = [];
  foreach ($sources as $source) {
    if (empty($source['url'])) {
      continue;
    }
    $title = !empty($source['title']) ? $source['title'] : $source['url'];
    $lines[] = '- [' . $title . '](' . $source['url'] . ')';
  }

  if (empty($lines)) {
    return (string) $answer;
  }

  $answer = rtrim((string) $answer);
  return $answer . "\n\nSources:\n" . implode("\n", $lines);
}

/**
 * Split one answer into body and source sections.
 */
function ai_assistants_chat_split_answer_sections($answer) {
  $answer = (string) $answer;
  $marker = "\n\nSources:\n";
  $pos = strpos($answer, $marker);
  if ($pos === FALSE) {
    return [
      'body' => $answer,
      'sources' => '',
    ];
  }

  return [
    'body' => substr($answer, 0, $pos),
    'sources' => substr($answer, $pos + strlen($marker)),
  ];
}

/**
 * Render one answer for chat history, with optional client-side streaming.
 */
function ai_assistants_chat_render_answer($answer, $stream = FALSE) {
  $sections = ai_assistants_chat_split_answer_sections($answer);
  $body = trim((string) $sections['body']);
  $sources = trim((string) $sections['sources']);

  $body_html = ai_assistants_chat_format_text($body);
  if ($stream) {
    $body_html = '<span class="chat-stream-text" data-stream-text="' . check_plain($body) . '" data-final-html="' . check_plain($body_html) . '">' . check_plain($body) . '</span>';
  }

  if ($sources === '') {
    return $body_html;
  }

  $sources_html = '<div class="chat-history-heading">' . check_plain(t('Sources')) . '</div>' . ai_assistants_chat_format_text($sources);
  $sources_classes = $stream ? 'chat-answer-sources is-hidden' : 'chat-answer-sources';

  return $body_html . '<div class="' . $sources_classes . '">' . $sources_html . '</div>';
}

/**
 * Chat form for assistant block.
 */
function ai_assistants_chat_form($form, &$form_state, $block_id = NULL, $block_config = []) {
  $asset_module = ai_assistants_chatbot_asset_module();
  $form['#attached']['css'][] = backdrop_get_path('module', $asset_module) . '/css/chat.css';
  $form['#attached']['js'][] = backdrop_get_path('module', $asset_module) . '/js/chat.js';

  $wrapper_id = 'chatbot-form-wrapper-' . preg_replace('/[^a-z0-9_-]+/', '-', strtolower((string) $block_id));
  $history_id = 'ai-assistant-chat-history-' . preg_replace('/[^a-z0-9_-]+/', '-', strtolower((string) $block_id));
  $form['#prefix'] = '<div id="' . check_plain($wrapper_id) . '">';
  $form['#suffix'] = '</div>';
  $form['#submit'] = ['ai_assistants_chat_form_submit'];

  $assistant_id = !empty($block_config['assistant_id']) ? (string) $block_config['assistant_id'] : '';
  $chat_instance_id = ai_assistants_block_instance_id($block_config);
  // Scope all transient UI state by block instance so separate chatbot blocks
  // can point at different assistants without sharing transcript or approvals.
  $history_key = ai_assistants_chat_session_key('history', $chat_instance_id);
  $status_key = ai_assistants_chat_session_key('status', $chat_instance_id);
  $thread_key = ai_assistants_chat_session_key('thread', $chat_instance_id);
  $history = !empty($_SESSION[$history_key]) && is_array($_SESSION[$history_key]) ? $_SESSION[$history_key] : [];
  $assistant = $assistant_id !== '' ? ai_assistants_load($assistant_id) : [];
  $stream_last = !empty($assistant['status_events'])
    && !empty($_SESSION['ai_assistants_chat_stream_last'][$chat_instance_id])
    && !empty($history);
  if ($stream_last) {
    unset($_SESSION['ai_assistants_chat_stream_last'][$chat_instance_id]);
  }
  $status_message = !empty($_SESSION[$status_key]) ? (string) $_SESSION[$status_key] : '';
  if ($status_message !== '') {
    unset($_SESSION[$status_key]);
  }

  $history_html = '<div id="' . check_plain($history_id) . '" class="chat-history-scroll" role="log" aria-live="polite" aria-relevant="additions text" aria-atomic="false">';
  if ($status_message !== '') {
    $history_html .= '<div class="ai-assistant-chat-status" role="status" aria-live="polite" aria-atomic="true">' . check_plain($status_message) . '</div>';
  }
  $history_count = count($history);
  foreach ($history as $index => $exchange) {
    $question = check_plain(isset($exchange['question']) ? $exchange['question'] : '');
    $answer = ai_assistants_chat_render_answer(
      isset($exchange['answer']) ? (string) $exchange['answer'] : '',
      $stream_last && $index === ($history_count - 1)
    );
    $meta = '';
    if (!empty($exchange['status'])) {
      $meta = '<div class="ai-assistant-chat-meta">' . check_plain(ucfirst(str_replace('_', ' ', (string) $exchange['status']))) . '</div>';
    }
    $history_html .= '<div class="chat-history-exchange">';
    $history_html .= '<div class="chat-history-question"><strong>Q:</strong> ' . $question . '</div>';
    $history_html .= '<div class="chat-history-answer"><strong>A:</strong> ' . $answer . '</div>';
    if ($meta !== '') {
      $history_html .= $meta;
    }
    $history_html .= '</div>';
  }
  $history_html .= '</div>';

  $form['history'] = [
    '#markup' => $history_html,
    '#weight' => -100,
  ];

  $form['assistant_id'] = [
    '#type' => 'hidden',
    '#value' => $assistant_id,
  ];
  $form['chat_instance_id'] = [
    '#type' => 'hidden',
    '#value' => $chat_instance_id,
  ];
  $form['chat_thread_key'] = [
    '#type' => 'hidden',
    '#value' => $thread_key,
  ];

  $pending = ai_assistants_chat_get_pending($chat_instance_id);
  if (!empty($pending) && !empty($pending['token'])) {
    $pending_label = !empty($pending['name']) ? check_plain((string) $pending['name']) : t('this tool');
    $form['approval'] = [
      '#type' => 'fieldset',
      '#title' => t('Approval needed'),
      '#weight' => -50,
    ];
    $form['approval']['pending_token'] = [
      '#type' => 'hidden',
      '#value' => (string) $pending['token'],
      '#parents' => ['pending_token'],
    ];
    $form['approval']['pending_agent_id'] = [
      '#type' => 'hidden',
      '#value' => !empty($pending['agent_id']) ? (string) $pending['agent_id'] : '',
      '#parents' => ['pending_agent_id'],
    ];
    $form['approval']['pending_thread_id'] = [
      '#type' => 'hidden',
      '#value' => !empty($pending['thread_id']) ? (string) $pending['thread_id'] : '',
      '#parents' => ['pending_thread_id'],
    ];
    $form['approval']['pending_question'] = [
      '#type' => 'hidden',
      '#value' => !empty($pending['question']) ? (string) $pending['question'] : '',
      '#parents' => ['pending_question'],
    ];
    $form['approval']['message'] = [
      '#markup' => '<p>' . t('Approval required before running @tool.', ['@tool' => $pending_label]) . '</p>',
    ];
    $form['approval']['approve'] = [
      '#type' => 'submit',
      '#value' => t('Approve'),
      '#name' => 'approve_pending',
      '#submit' => ['ai_assistants_chat_approve_submit'],
      '#ajax' => [
        'callback' => 'ai_assistants_chat_ajax_response',
        'wrapper' => $wrapper_id,
        'method' => 'replace',
        'effect' => 'fade',
      ],
      '#validate' => [],
      '#limit_validation_errors' => FALSE,
    ];
    $form['approval']['deny'] = [
      '#type' => 'submit',
      '#value' => t('Deny'),
      '#name' => 'deny_pending',
      '#submit' => ['ai_assistants_chat_deny_submit'],
      '#ajax' => [
        'callback' => 'ai_assistants_chat_ajax_response',
        'wrapper' => $wrapper_id,
        'method' => 'replace',
        'effect' => 'fade',
      ],
      '#validate' => [],
      '#limit_validation_errors' => FALSE,
    ];
  }

  $form['query'] = [
    '#type' => 'textarea',
    '#title' => t('Ask me a question'),
    '#title_display' => 'invisible',
    '#attributes' => [
      'placeholder' => t('Ask me a question'),
      'class' => ['chat-form-query'],
    ],
    '#required' => FALSE,
    '#rows' => 3,
    '#default_value' => '',
  ];

  $form['actions'] = [
    '#type' => 'actions',
  ];
  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Send'),
    '#ajax' => [
      'callback' => 'ai_assistants_chat_ajax_response',
      'wrapper' => $wrapper_id,
      'method' => 'replace',
      'effect' => 'fade',
    ],
    '#attributes' => [
      'class' => ['ai-assistant-chat-send', 'chat-form-send'],
    ],
  ];
  $form['actions']['clear_history_submit'] = [
    '#type' => 'submit',
    '#value' => t('Clear History'),
    '#name' => 'clear_history',
    '#ajax' => [
      'callback' => 'ai_assistants_chat_ajax_response',
      'wrapper' => $wrapper_id,
      'method' => 'replace',
      'effect' => 'fade',
    ],
    '#validate' => [],
    '#limit_validation_errors' => FALSE,
    '#submit' => ['ai_assistants_chat_clear_history_submit'],
    '#attributes' => [
      'class' => ['chat-form-clear-history'],
    ],
  ];

  return $form;
}

/**
 * Ajax callback for assistant chat form updates.
 */
function ai_assistants_chat_ajax_response($form, &$form_state) {
  return $form;
}

/**
 * Clears assistant chat history.
 */
function ai_assistants_chat_clear_history_submit($form, &$form_state) {
  $chat_instance_id = !empty($form_state['values']['chat_instance_id']) ? (string) $form_state['values']['chat_instance_id'] : '';
  $thread_key = !empty($form_state['values']['chat_thread_key']) ? (string) $form_state['values']['chat_thread_key'] : '';
  if ($chat_instance_id !== '') {
    $_SESSION[ai_assistants_chat_session_key('history', $chat_instance_id)] = [];
    unset($_SESSION['ai_assistants_chat_stream_last'][$chat_instance_id]);
    ai_assistants_chat_clear_pending($chat_instance_id);
    if ($thread_key !== '' && !empty($_SESSION[$thread_key])) {
      ai_assistants_clear_thread((string) $_SESSION[$thread_key]);
      unset($_SESSION[$thread_key]);
    }
  }
  $form_state['rebuild'] = TRUE;
  $form_state['values']['query'] = '';
  $form_state['input']['query'] = '';
}

/**
 * Approves a pending assistant action.
 */
function ai_assistants_chat_approve_submit($form, &$form_state) {
  $assistant_id = !empty($form_state['values']['assistant_id']) ? (string) $form_state['values']['assistant_id'] : '';
  $chat_instance_id = !empty($form_state['values']['chat_instance_id']) ? (string) $form_state['values']['chat_instance_id'] : '';
  $status_key = ai_assistants_chat_session_key('status', $chat_instance_id);
  $pending_token = !empty($form_state['values']['pending_token']) ? (string) $form_state['values']['pending_token'] : '';
  $pending_agent_id = !empty($form_state['values']['pending_agent_id']) ? (string) $form_state['values']['pending_agent_id'] : '';
  $pending_thread_id = !empty($form_state['values']['pending_thread_id']) ? (string) $form_state['values']['pending_thread_id'] : '';
  $pending_question = !empty($form_state['values']['pending_question']) ? html_entity_decode((string) $form_state['values']['pending_question'], ENT_QUOTES, 'UTF-8') : '';
  if (empty($assistant_id) || $pending_token === '' || $pending_agent_id === '') {
    $_SESSION[$status_key] = t('No pending action to approve.');
    $form_state['rebuild'] = TRUE;
    return;
  }

  $result = ai_assistants_resume($assistant_id, $pending_token, TRUE, [
    'thread_id' => $pending_thread_id !== '' ? $pending_thread_id : NULL,
    'agent_id' => $pending_agent_id,
  ]);

  if (empty($result['error']) && (!empty($result['status']) && $result['status'] === 'ok') && $pending_thread_id !== '') {
    $assistant = ai_assistants_load($assistant_id);
    if (!empty($assistant)) {
      $result = ai_assistants_continue_after_approval($assistant_id, $assistant, $result, $pending_agent_id, $pending_thread_id, [
        'thread_id' => $pending_thread_id,
        'agent_id' => $pending_agent_id,
      ]);
    }
  }

  ai_assistants_chat_append_result($chat_instance_id, t('Approve'), $result);
  if (!empty($result['status']) && $result['status'] === 'needs_approval' && !empty($result['token'])) {
    ai_assistants_chat_set_pending($chat_instance_id, [
      'token' => (string) $result['token'],
      'agent_id' => !empty($result['pending_agent_id']) ? (string) $result['pending_agent_id'] : (!empty($result['agent_id']) ? (string) $result['agent_id'] : ''),
      'name' => function_exists('ai_assistants_pending_tools_summary') ? ai_assistants_pending_tools_summary($result) : (!empty($result['pending']['name']) ? (string) $result['pending']['name'] : ''),
      'thread_id' => $pending_thread_id,
      'question' => $pending_question,
    ]);
  }
  else {
    ai_assistants_chat_clear_pending($chat_instance_id);
  }
  $_SESSION[$status_key] = !empty($result['error'])
    ? (string) $result['error']
    : ((!empty($result['status']) && $result['status'] === 'needs_approval') ? t('Action approved. Another step needs approval.') : t('Action approved.'));
  $form_state['values']['query'] = '';
  $form_state['input']['query'] = '';
  $form_state['rebuild'] = TRUE;
}

/**
 * Denies a pending assistant action.
 */
function ai_assistants_chat_deny_submit($form, &$form_state) {
  $assistant_id = !empty($form_state['values']['assistant_id']) ? (string) $form_state['values']['assistant_id'] : '';
  $chat_instance_id = !empty($form_state['values']['chat_instance_id']) ? (string) $form_state['values']['chat_instance_id'] : '';
  $status_key = ai_assistants_chat_session_key('status', $chat_instance_id);
  $pending_token = !empty($form_state['values']['pending_token']) ? (string) $form_state['values']['pending_token'] : '';
  if (empty($assistant_id) || $pending_token === '') {
    $_SESSION[$status_key] = t('No pending action to deny.');
    $form_state['rebuild'] = TRUE;
    return;
  }

  if (function_exists('ai_agents_clear_pending_run')) {
    ai_agents_clear_pending_run($pending_token);
  }
  ai_assistants_chat_clear_pending($chat_instance_id);
  ai_assistants_chat_append_result($chat_instance_id, t('Deny'), [
    'status' => 'denied',
    'answer' => t('Approval denied. No changes were made.'),
    'error' => NULL,
  ]);
  $_SESSION[$status_key] = t('Action denied.');
  $form_state['rebuild'] = TRUE;
}

/**
 * Sends assistant chat message.
 */
function ai_assistants_chat_form_submit($form, &$form_state) {
  if (!empty($form_state['values']['clear_history']) || !empty($form_state['input']['clear_history'])) {
    ai_assistants_chat_clear_history_submit($form, $form_state);
    return;
  }

  $assistant_id = !empty($form_state['values']['assistant_id']) ? (string) $form_state['values']['assistant_id'] : '';
  $chat_instance_id = !empty($form_state['values']['chat_instance_id']) ? (string) $form_state['values']['chat_instance_id'] : '';
  $status_key = ai_assistants_chat_session_key('status', $chat_instance_id);
  $thread_key = !empty($form_state['values']['chat_thread_key']) ? (string) $form_state['values']['chat_thread_key'] : '';
  $query = trim((string) $form_state['values']['query']);

  if ($assistant_id === '') {
    $_SESSION[$status_key] = t('Select an assistant in the block settings first.');
    $form_state['rebuild'] = TRUE;
    return;
  }

  if ($query === '') {
    $_SESSION[$status_key] = t('Ask a question before sending.');
    $form_state['rebuild'] = TRUE;
    return;
  }

  $thread_id = !empty($_SESSION[$thread_key]) ? $_SESSION[$thread_key] : NULL;

  // If there is a pending approval and the user typed an approval message,
  // resume the paused run instead of starting a new agent call.
  $pending = ai_assistants_chat_get_pending($chat_instance_id);
  if (!empty($pending['token']) && !empty($pending['agent_id']) && ai_assistants_is_approval_text($query)) {
    $approval_values = [
      'assistant_id'      => $assistant_id,
      'chat_instance_id'  => $chat_instance_id,
      'pending_token'     => $pending['token'],
      'pending_agent_id'  => $pending['agent_id'],
      'pending_thread_id' => !empty($pending['thread_id']) ? $pending['thread_id'] : '',
      'pending_question'  => !empty($pending['question']) ? $pending['question'] : '',
    ];
    $approval_form_state = ['values' => $approval_values, 'rebuild' => FALSE];
    ai_assistants_chat_approve_submit($form, $approval_form_state);
    $form_state['rebuild'] = TRUE;
    return;
  }

  // Fall back to the most recent DB thread for this user+assistant when the
  // session thread key has been lost (e.g. after navigating away and back).
  if ($thread_id === NULL) {
    global $user;
    if (!empty($user->uid) && $assistant_id !== '') {
      $row = db_query(
        "SELECT thread_id FROM {ai_assistants_thread} WHERE assistant_id = :aid AND uid = :uid ORDER BY updated DESC LIMIT 1",
        [':aid' => $assistant_id, ':uid' => (int) $user->uid]
      )->fetchField();
      if ($row) {
        $thread_id = (string) $row;
        $_SESSION[$thread_key] = $thread_id;
      }
    }
  }

  $result = ai_assistants_send($assistant_id, $query, [
    'thread_id' => $thread_id,
  ]);

  if (!empty($result['thread_id'])) {
    $_SESSION[$thread_key] = $result['thread_id'];
  }

  $answer = '';
  $status = !empty($result['status']) ? (string) $result['status'] : 'ok';
  if ($status === 'needs_approval') {
    $pending_summary = function_exists('ai_assistants_pending_tools_summary')
      ? ai_assistants_pending_tools_summary($result)
      : (!empty($result['pending']['name']) ? (string) $result['pending']['name'] : '');
    $pending_count = !empty($result['pending_tools']) && is_array($result['pending_tools']) ? count($result['pending_tools']) : 1;
    if ($pending_summary !== '' && $pending_count > 1) {
      $answer = t('Approval required before running @count actions: @tools. Approving runs ALL of them.', ['@count' => $pending_count, '@tools' => $pending_summary]);
    }
    elseif ($pending_summary !== '') {
      $answer = t('Approval required before running @tool.', ['@tool' => $pending_summary]);
    }
    else {
      $answer = t('Approval required before running this request.');
    }
  }
  elseif (!empty($result['answer'])) {
    $answer = (string) $result['answer'];
  }
  elseif (!empty($result['error'])) {
    $answer = (string) $result['error'];
  }
  else {
    $answer = t('The assistant did not return an answer.');
  }

  $rag_sources = !empty($result['tool_calls']) && is_array($result['tool_calls'])
    ? ai_assistants_extract_rag_sources($result['tool_calls'])
    : [];

  if ($status === 'ok' && !empty($rag_sources)) {
    // Only render links that came from actual RAG tool output, not links the
    // model may have invented while composing the answer text.
    $answer = ai_assistants_sanitize_answer_links($answer, $rag_sources);
    $answer = ai_assistants_append_rag_sources($answer, $rag_sources);
  }

  if ($status === 'needs_approval' && !empty($result['token'])) {
    ai_assistants_chat_set_pending($chat_instance_id, [
      'token' => (string) $result['token'],
      'agent_id' => !empty($result['pending_agent_id']) ? (string) $result['pending_agent_id'] : (!empty($result['agent_id']) ? (string) $result['agent_id'] : ''),
      'name' => function_exists('ai_assistants_pending_tools_summary') ? ai_assistants_pending_tools_summary($result) : (!empty($result['pending']['name']) ? (string) $result['pending']['name'] : ''),
      'thread_id' => !empty($result['thread_id']) ? (string) $result['thread_id'] : '',
      'question' => html_entity_decode($query, ENT_QUOTES, 'UTF-8'),
    ]);
  }
  else {
    ai_assistants_chat_clear_pending($chat_instance_id);
  }

  ai_assistants_chat_append_result($chat_instance_id, $query, [
    'answer' => $answer,
    'status' => $status,
    'error' => !empty($result['error']) ? $result['error'] : NULL,
  ]);
  $assistant = ai_assistants_load($assistant_id);
  if (!empty($assistant['status_events'])) {
    $_SESSION['ai_assistants_chat_stream_last'][$chat_instance_id] = TRUE;
  }

  if ($status === 'error' && !empty($result['error'])) {
    $_SESSION[$status_key] = (string) $result['error'];
  }
  elseif ($status === 'needs_approval') {
    $_SESSION[$status_key] = t('Action pending approval.');
  }
  else {
    $_SESSION[$status_key] = t('Response received.');
  }

  $form_state['values']['query'] = '';
  $form_state['input']['query'] = '';
  $form_state['rebuild'] = TRUE;
}

/**
 * Appends transcript entry for assistant chat.
 */
function ai_assistants_chat_append_result($chat_instance_id, $question, array $result) {
  $history_key = ai_assistants_chat_session_key('history', $chat_instance_id);
  if (!isset($_SESSION[$history_key]) || !is_array($_SESSION[$history_key])) {
    $_SESSION[$history_key] = [];
  }
  $status = !empty($result['status']) ? (string) $result['status'] : 'ok';
  $answer = !empty($result['answer']) ? (string) $result['answer'] : '';
  if ($answer === '' && !empty($result['error'])) {
    $answer = (string) $result['error'];
  }
  if ($answer === '') {
    $answer = ($status === 'ok') ? t('The assistant did not return an answer.') : t('No response.');
  }

  $_SESSION[$history_key][] = [
    'question' => $question,
    'answer' => $answer,
    'status' => $status,
  ];
}
