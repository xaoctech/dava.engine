#include "BaseParticleEditorContentWidget.h"

const QChar BaseParticleEditorContentWidget::DEGREE_MARK_CHARACTER = QChar(0x00B0);

BaseParticleEditorContentWidget::BaseParticleEditorContentWidget()
{
	emitter = NULL;
}

int BaseParticleEditorContentWidget::ConvertFromPlaybackSpeedToSliderValue(float32 playbackSpeed)
{
	playbackSpeed = Clamp(playbackSpeed, PARTICLE_EMITTER_MIN_PLAYBACK_SPEED, PARTICLE_EMITTER_MAX_PLAYBACK_SPEED);
	
	// Our scale is logarithmic.
	uint playbackSpeedInt = (uint)(playbackSpeed / PARTICLE_EMITTER_MIN_PLAYBACK_SPEED);
	uint logValue = -1;
	while (playbackSpeedInt)
	{
		logValue++;
		playbackSpeedInt >>= 1;
	}
	
	return logValue;
}

float BaseParticleEditorContentWidget::ConvertFromSliderValueToPlaybackSpeed(int sliderValue)
{
	// Our scale is logarithmic.
	uint scaleFactor = (1 << sliderValue);
	return PARTICLE_EMITTER_MIN_PLAYBACK_SPEED * scaleFactor;
}