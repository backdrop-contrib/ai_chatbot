# AI Chatbot

`ai_chatbot` is the UI/block module that renders a page chatbot for one selected assistant.

It owns:

- the chatbot block plugin
- block instance settings
- layout-form UX for admin label/description
- AJAX chat form submission
- chat transcript rendering
- approval buttons
- chatbot CSS and JavaScript assets

Main files:

- [ai_chatbot.module](ai_chatbot.module)
- [includes/ai_chatbot.block.inc](includes/ai_chatbot.block.inc)
- [css/chat.css](css/chat.css)
- [js/chat.js](js/chat.js)

Key idea:

- each placed block gets its own `instance_id`
- that `instance_id` scopes history, pending approval state, and thread pointer
- two chatbot blocks can therefore use different assistants without sharing chat state

This module depends on `ai_assistants` for the actual assistant runtime.

## Credits

- Created for Backdrop CMS by [Justin Keiser](https://github.com/keiserjb).
- Developed with AI assistance.
