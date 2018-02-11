#include "audio-oscilloscope.h"

// Returns trigger index or -1 if not found yet.
int FindOscilloscopeTrigger(float* Samples, int Length) {
    const float Level = 0; // Value to trigger at (on rising slope)
    for (int I = 0; I < Length; I++) {
        const float V1 = Samples[I];
        const float V0 = I > 0 ? Samples[I-1] : 1;
        if (V0 < Level && V1 >= Level) return I;
    }
    return 0;
}

void InitScope(scope* Scope) {

    glGenBuffers(1, &Scope->TexBuf);
    glGenTextures(1, &Scope->Tex);

    glBindTexture(GL_TEXTURE_BUFFER, Scope->Tex);
    glBindBuffer(GL_TEXTURE_BUFFER, Scope->TexBuf);
    glBufferData(GL_TEXTURE_BUFFER,
        BUFFER_SIZE * sizeof(float),
        NULL, GL_DYNAMIC_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, Scope->TexBuf);
}

void TickOscilloscope(ringbuffer* AudioTap, scope* Scope, GLenum TexUnit) {

    while (GetRingBufferReadAvailable(AudioTap) >= 1) {
        audio_block Block;
        ReadRingBuffer(AudioTap, &Block, 1);

        bool ResetIndex = (Scope->WriteIndex + Block.Length) > (BUFFER_SIZE_2X);
        if (ResetIndex) {
            int TriggerPoint = FindOscilloscopeTrigger(Scope->LocalBuf, BUFFER_SIZE_2X);
            if (Scope->WriteIndex - TriggerPoint > BUFFER_SIZE) {
                glBindBuffer(GL_TEXTURE_BUFFER, Scope->TexBuf);
                glBufferSubData(GL_TEXTURE_BUFFER,
                    0,
                    BUFFER_SIZE*sizeof(float),
                    &Scope->LocalBuf[TriggerPoint]);

                // Move the data after what we uploaded to the beginning of the TempBuffer
                // and set AudioBufIndexes to its length.
                size_t AmountToCopy = Scope->WriteIndex - (TriggerPoint+BUFFER_SIZE);
                memmove(&Scope->LocalBuf[0],
                        &Scope->LocalBuf[TriggerPoint+BUFFER_SIZE],
                        AmountToCopy*sizeof(float));

                Scope->WriteIndex = AmountToCopy;
            } else {
                Scope->WriteIndex = 0;
            }
        }

        for (int I = 0; I < Block.Length; I++) {
            Scope->LocalBuf[Scope->WriteIndex++] = Block.Samples[I];
        }

        free(Block.Samples);
        free(Block.Freqs);
    }

    glActiveTexture(TexUnit);
    glBindTexture(GL_TEXTURE_BUFFER, Scope->Tex);
}
