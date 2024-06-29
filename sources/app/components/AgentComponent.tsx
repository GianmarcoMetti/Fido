import React, { useState } from 'react';
import { Agent } from '../agent/Agent'; 

const AgentComponent = () => {
    const agent = new Agent();
    const [triggerInput, setTriggerInput] = useState('');
    const [state, setState] = useState(agent.use());

    const handleInputChange = (event) => {
        setTriggerInput(event.target.value);
    };

    const handleAddTrigger = () => {
        if (triggerInput.trim()) {
            agent.addtrigger(triggerInput.trim());
            setTriggerInput('');
        }
    };

    return (
        <div>
            <h1>Agent Triggers</h1>
            <input
                type="text"
                value={triggerInput}
                onChange={handleInputChange}
                placeholder="Enter trigger"
            />
            <button onClick={handleAddTrigger}>Add Trigger</button>
            <div>
                <h2>Current Triggers</h2>
                <p>{state.triggerstr}</p>
            </div>
        </div>
    );
};

export default AgentComponent;
