'use strict';

const countLabels = [
  ['blocked', 'Blocked'],
  ['sanitized', 'Sanitized'],
  ['compute_privately', 'Computed privately'],
  ['prompt', 'Prompted'],
  ['allow', 'Allowed'],
];

function text(value) {
  return value === undefined || value === null ? '' : String(value);
}

function renderCounts(counts) {
  const container = document.getElementById('counts');
  container.textContent = '';
  for (const [key, label] of countLabels) {
    const card = document.createElement('div');
    card.className = 'card';
    const name = document.createElement('div');
    name.className = 'muted';
    name.textContent = label;
    const value = document.createElement('div');
    value.className = 'value';
    value.textContent = text(counts[key] || 0);
    card.append(name, value);
    container.append(card);
  }
}

function renderActivity(activities) {
  const container = document.getElementById('activity-list');
  container.textContent = '';
  if (!activities.length) {
    const empty = document.createElement('p');
    empty.className = 'muted empty';
    empty.textContent = 'No privacy activity has been recorded in this profile.';
    container.append(empty);
    return;
  }

  for (const activity of activities.slice().reverse()) {
    const item = document.createElement('article');
    item.className = 'activity';
    const time = new Date(activity.timestamp).toLocaleString();
    const summary = [activity.decision, activity.surface].join(' · ');
    const details = [
      activity.requester,
      activity.requester_type,
      activity.private_input_type === 'none' ? '' :
          `${activity.private_input_type} input count ${activity.private_input_count}`,
      activity.disclosed_type === 'none' ? '' :
          `disclosed ${activity.disclosed_type}`,
      activity.reason,
    ].filter(Boolean).join(' · ');
    const label = document.createElement('strong');
    label.textContent = summary;
    const when = document.createElement('span');
    when.className = 'muted';
    when.textContent = time;
    const detailLabel = document.createElement('strong');
    detailLabel.textContent = 'Requester';
    const detail = document.createElement('span');
    detail.className = 'muted';
    detail.textContent = details;
    item.append(label, when, detailLabel, detail);
    container.append(item);
  }
}

window.privyPrivacy = {
  onState(state) {
    document.getElementById('protection-status').textContent =
        state.protection_status;
    document.getElementById('provider-status').textContent =
        state.provider_status;
    document.getElementById('request-count').textContent =
        text(state.activities.length);
    renderCounts(state.counts);
    renderActivity(state.activities);
  },

  onFrequencyCapResult(allowed) {
    const result = document.getElementById('frequency-result');
    result.textContent = allowed ?
        'Frequency-cap result: allowed.' :
        'Frequency-cap result: blocked.';
  },
};

document.getElementById('run-frequency').addEventListener(
    'click', () => chrome.send('runFrequencyCap'));
document.getElementById('clear-activity').addEventListener(
    'click', () => chrome.send('clearActivity'));
chrome.send('getPrivacyState');
