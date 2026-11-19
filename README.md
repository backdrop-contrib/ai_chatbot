# AI Chatbot

Place a chat interface on a Backdrop page and choose the AI assistant that
answers there. Each placement maintains its own chat state.

## Requirements

- PHP 8.2 or later
- `ai_assistants` module

## Installation

Install this module using the official [Backdrop CMS instructions](https://backdropcms.org/user-guide/modules).

1. Configure an assistant at `admin/config/ai/ai-assistants`.
2. Edit a layout and add an **AI Chatbot** block.
3. Choose the **Assistant**, enter an optional visitor-facing **Block title**,
   and save the block and layout.

Enable the bundled **AI Chatbot Personas** submodule to set a voice or style
for an individual block.

Find setup, examples, and troubleshooting in the [project wiki](https://github.com/backdrop-contrib/ai_chatbot/wiki).

## Developer reference

`ai_chatbot` renders the chat UI for the selected assistant.

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

## Issues

Bugs and feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/ai_chatbot/issues).

## Current Maintainer

[Justin Keiser](https://github.com/keiserjb)

## Credits

- Created for Backdrop CMS by [Justin Keiser](https://github.com/keiserjb).
- Developed with AI assistance.

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
