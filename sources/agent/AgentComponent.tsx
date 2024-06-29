import React, { useState } from 'react';
import { View, Text, TextInput, Button, StyleSheet } from 'react-native';
import { Agent } from '../agent/Agent'; // Adjust the relative path as needed

const AgentComponent = () => {
    const agent = new Agent();
    const [triggerInput, setTriggerInput] = useState('');
    const [state, setState] = useState(agent.use());

    const handleInputChange = (text) => {
        setTriggerInput(text);
    };

    const handleAddTrigger = () => {
        if (triggerInput.trim()) {
            agent.addtrigger(triggerInput.trim());
            setTriggerInput('');
        }
    };

    return (
        <View style={styles.container}>
            <Text style={styles.title}>Agent Triggers</Text>
            <TextInput
                style={styles.input}
                value={triggerInput}
                onChangeText={handleInputChange}
                placeholder="Enter trigger"
            />
            <Button title="Add Trigger" onPress={handleAddTrigger} />
            <View style={styles.triggersContainer}>
                <Text style={styles.triggersTitle}>Current Triggers:</Text>
                <Text>{state.triggerstr}</Text>
            </View>
        </View>
    );
};

const styles = StyleSheet.create({
    container: {
        padding: 20,
    },
    title: {
        fontSize: 24,
        marginBottom: 10,
    },
    input: {
        height: 40,
        borderColor: 'gray',
        borderWidth: 1,
        marginBottom: 10,
        paddingHorizontal: 10,
    },
    triggersContainer: {
        marginTop: 20,
    },
    triggersTitle: {
        fontSize: 18,
        marginBottom: 5,
    },
});

export default AgentComponent;
