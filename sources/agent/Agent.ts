import * as React from 'react';
import { AsyncLock } from "../utils/lock";
import { imageDescription, llamaFind, openAIFind } from "./imageDescription";
import { startAudio } from '../modules/openai';

type AgentState = {
    lastDescription?: string;
    answer?: string;
    snakes?: string;
    loading: boolean;
    log?: string[];
    trigger: string[];
    triggerstr?: string;
}

export class Agent {
    #lock = new AsyncLock();
    #photos: { photo: Uint8Array, description: string }[] = [];
    #state: AgentState = { loading: false, trigger: ['Snakes'] };
    #stateCopy: AgentState = { loading: false, trigger: ['Snakes'] };
    #stateListeners: (() => void)[] = [];

    // Cache to store frequently used results
    private cache = new Cache();

    async addPhoto(photos: Uint8Array[]) {
        await this.#lock.inLock(async () => {
            let lastDescription: string | null = null;

            // Process photos in parallel
            const descriptionPromises = photos.map(async (p) => {
                console.log('Processing photo', p.length);
                let description = await imageDescription(p);
                console.log('Description', description);

                // Preprocess description to remove unnecessary information
                this.#photos.push({ photo: p, description: preprocessDescription(description) });
                return description;
            });

            const descriptions = await Promise.all(descriptionPromises);
            lastDescription = descriptions[descriptions.length - 1];

            if (lastDescription) {
                this.#state.lastDescription = lastDescription;
                this.checkTriggers(lastDescription); // Check for triggers in the description
                this.#notify();
            }
        });
    }

    async answer(question: string, snakes: boolean) {
        if (!snakes) {
            if (this.#state.loading) {
                return;
            }
            this.#state.loading = true;
            this.#notify();
        }

        await this.#lock.inLock(async () => {
            let combined = '';
            let i = 0;
            for (let p of this.#photos) {
                combined += `\n\nImage #${i}\n\n${p.description}`;
                i++;
            }

            // Add context to the description
            const contextDescription = `
                The images described are taken from a dog's point of view as the dog is wearing a microcamera. 
                The owner of the dog is asking the following question:
            ` + combined;

            // Create a cache key based on the question and context description
            const cacheKey = `${question}-${contextDescription}`;
            let answer = this.cache.get(cacheKey);

            // If no cached answer, fetch the answer and cache it
            if (!answer) {
                answer = await llamaFind(question, contextDescription);
                this.cache.set(cacheKey, answer);
            }

            if (snakes) {
                let dateTime = new Date();
                this.#state.snakes = answer;
                this.#state.log?.push(dateTime.toString() + answer);
            } else {
                this.#state.answer = answer;
                this.#state.loading = false;
                this.#notify();
            }
        });
    }

    addtrigger(trigger: string) {
        const index = this.#state.trigger.indexOf(trigger);
        if (index > -1) {
            this.#state.trigger.splice(index, 1);
        } else {
            this.#state.trigger.push(trigger);
        }
        this.printtrigger();
    }

    returnTrigger() {
        return this.#state.trigger;
    }

    printtrigger() {
        this.#state.triggerstr = "";
        var l = this.#state.trigger.length;
        for (var i = 0; i < l; i++) {
            this.#state.triggerstr += this.#state.trigger[i];
            if (i != l - 1) {
                this.#state.triggerstr += ", ";
            }
        }
    }

    #notify = () => {
        this.#stateCopy = { ...this.#state };
        for (let l of this.#stateListeners) {
            l();
        }
    }

    checkTriggers(description: string) {
        this.#state.trigger.forEach(trigger => {
            if (description.includes(trigger)) {
                this.sendAlert(trigger, description);
            }
        });
    }

    sendAlert(trigger: string, description: string) {
        // Custom logic for different alerts based on the trigger
        if (trigger === 'Snakes') {
            alert(`Alert! The description contains the trigger '${trigger}': ${description}`);
            // Add any other notification logic here (e.g., sending an email, logging, etc.)
        } else {
            alert(`Trigger detected: ${trigger}`);
            // Add custom handling for other triggers here
        }
    }

    use() {
        const [state, setState] = React.useState(this.#stateCopy);
        React.useEffect(() => {
            const listener = () => setState(this.#stateCopy);
            this.#stateListeners.push(listener);
            return () => {
                this.#stateListeners = this.#stateListeners.filter(l => l !== listener);
            }
        }, []);
        return state;
    }
}

// Function to preprocess descriptions
function preprocessDescription(description: string): string {
    // Implement any specific preprocessing logic here
    return description.replace(/irrelevant information/gi, '');
}

// Cache class to store and retrieve frequently used results
class Cache {
    private cache: Map<string, string> = new Map();

    get(key: string): string | undefined {
        return this.cache.get(key);
    }

    set(key: string, value: string): void {
        this.cache.set(key, value);
    }
}
