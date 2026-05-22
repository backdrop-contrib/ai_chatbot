# AI Chatbot Personas

Admin-configured persona controls for AI Chatbot blocks. Lets you assign a voice or tone to each chatbot block instance — Concise, Tutor, Storyteller, Skeptic, Trippy, Unreliable Narrator, or a fully custom style prompt. The persona adjusts tone and presentation only; assistant system prompts and guardrails remain in effect.

## Requirements

- `ai_chatbot` module
- `ai_assistants` module

## Installation

- Install this module using the official [Backdrop CMS instructions](https://backdropcms.org/user-guide/modules).

## Configuration

1. Enable this module.
2. Go to your layout configuration and edit an AI Chatbot block.
3. Expand the **Persona** fieldset.
4. Select a built-in persona or choose **Custom** and enter your own style instructions.
5. Save the block. The persona applies immediately to all new chat requests on that block.

## Notes

- Each chatbot block instance stores its own persona. Multiple blocks on the same site can use different personas.
- Persona settings are stored in `ai_chatbot_personas.settings` keyed by block instance ID, separate from the assistant configuration.
- The `Unreliable Narrator` and `Trippy` personas are creative/entertainment modes and may produce imaginative responses. Use them on appropriate sites only.

## Issues

Bugs and feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/ai_chatbot_personas/issues).

## Current Maintainer

[Justin Keiser](https://github.com/keiserjb)

## Credits

- Created for Backdrop CMS by [Justin Keiser](https://github.com/keiserjb).

- Developed with AI assistance.

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
