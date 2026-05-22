(function ($, Backdrop) {
  'use strict';

  function scrollToLatestExchangeTop($history) {
    var $latest = $history.find('.chat-history-exchange').last();
    var top;

    if (!$history.length || !$latest.length) {
      return;
    }

    top = $latest.position().top + $history.scrollTop() - 8;
    $history.scrollTop(Math.max(0, top));
  }

  function initializeHistoryPosition($history) {
    var hasNewResponse = $history.find('.chat-stream-text').length > 0
      || $history.find('.ai-assistant-chat-status, .openai-assistant-chat-status').length > 0;

    if (hasNewResponse) {
      scrollToLatestExchangeTop($history);
      return;
    }

    $history.scrollTop($history[0].scrollHeight);
  }

  function escapeHtml(text) {
    return String(text)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }

  function streamWords($el, text, done) {
    var tokens = String(text).match(/\S+\s*/g) || [];
    var index = 0;

    $el.html('');

    (function next() {
      if (index >= tokens.length) {
        if (typeof done === 'function') {
          done();
        }
        return;
      }

      $el.html(escapeHtml(tokens.slice(0, index + 1).join('')).replace(/\n/g, '<br>'));
      index++;
      window.setTimeout(next, 70);
    })();
  }

  Backdrop.behaviors.aiAssistantChat = {
    attach: function (context) {
      var $context = $(context);

      $context.find('.chat-history-scroll').once('ai-assistant-scroll').each(function () {
        var $history = $(this);
        $history.attr({
          'role': 'log',
          'aria-live': 'polite',
          'aria-relevant': 'additions text',
          'aria-atomic': 'false'
        });
        initializeHistoryPosition($history);
      });

      $context.find('.ai-assistant-chat-status').once('ai-assistant-status-a11y').attr({
        'role': 'status',
        'aria-live': 'polite',
        'aria-atomic': 'true'
      });

      $context.find('.chat-stream-text').once('ai-assistant-stream').each(function () {
        var $textEl = $(this);
        var sourceText = $textEl.attr('data-stream-text') || $textEl.text();
        var finalHtml = $textEl.attr('data-final-html') || '';
        var $sources = $textEl.siblings('.chat-answer-sources');
        var $history = $textEl.closest('.chat-history-scroll');
        var $status = $textEl.closest('form').find('.ai-assistant-chat-status');

        if ($sources.length) {
          $sources.addClass('is-hidden');
        }
        if ($status.length) {
          $status.text(Backdrop.t('Assistant response loading.'));
        }
        if ($history.length) {
          scrollToLatestExchangeTop($history);
        }

        streamWords($textEl, sourceText, function () {
          if (finalHtml !== '') {
            $textEl.replaceWith(finalHtml);
          }
          if ($sources.length) {
            $sources.removeClass('is-hidden');
          }
          if ($status.length) {
            $status.text(Backdrop.t('Assistant response complete.'));
          }
        });
      });

      $context.find('.chat-form-query').once('chat-enter-submit').each(function () {
        var $input = $(this);
        $input.on('keydown', function (e) {
          if (e.isComposing || e.keyCode === 229) {
            return;
          }
          var isEnter = (e.key === 'Enter' || e.which === 13);
          if (isEnter && !e.shiftKey) {
            e.preventDefault();
            var $form = $input.closest('form');
            var $btn  = $form.find('.chat-form-send:enabled:visible').first();
            if ($btn.length) {
              $btn.trigger('mousedown').trigger('click');
            } else {
              $form.trigger('submit');
            }
          }
        });
      });

      $context.find('.chat-toggle-button').once('chat-toggle').each(function () {
        var $button = $(this);
        var $block  = $button.closest('.block-ai-chatbot, .block-ai-chatbot-chat, .block-ai-chatbot-ai-chatbot-chat-form, .block-ai-assistants-chat, .block-ai-assistant-chat, .block-ai-assistants-ai-assistants-chat-form, .block-ai-assistant-ai-assistant-chat-form, .block-openai-assistant-chat, .block-openai-assistant-openai-assistant-chat-form, .block');
        var $panel = $block.find('.chatbot-inner').first();

        if (!$panel.length) {
          return;
        }

        if (!$panel.attr('id')) {
          $panel.attr('id', 'ai-assistant-chatbot-panel');
        }
        $button.attr('aria-controls', $panel.attr('id'));
        var storageKey = 'ai-assistant-collapsed:' + $panel.attr('id');

        var collapseValue = localStorage.getItem(storageKey);
        if (collapseValue === null) {
          collapseValue = localStorage.getItem('openai-assistant-collapsed');
        }
        var isCollapsed = collapseValue === 'true';
        $block.toggleClass('collapsed', isCollapsed);
        $panel.prop('hidden', isCollapsed);
        $button.attr('aria-expanded', isCollapsed ? 'false' : 'true');
        $button.text(isCollapsed ? '💬 ' + Backdrop.t('Open Assistant') : '💬 ' + Backdrop.t('Assistant'));

        $button.on('click', function (e) {
          e.preventDefault();
          $block.toggleClass('collapsed');
          var now = $block.hasClass('collapsed');
          localStorage.setItem(storageKey, now);
          localStorage.setItem('openai-assistant-collapsed', now);
          $panel.prop('hidden', now);
          $button.attr('aria-expanded', now ? 'false' : 'true');
          $button.text(now ? '💬 ' + Backdrop.t('Open Assistant') : '💬 ' + Backdrop.t('Assistant'));

          if (!now) {
            var $history = $block.find('.chat-history-scroll');
            if ($history.length) {
              $history.scrollTop($history[0].scrollHeight);
            }
          }
        });
      });
    }
  };
})(jQuery, Backdrop);
