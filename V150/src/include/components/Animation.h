// Animation.h — Logic component. Frame-based sprite animation.
// Works alongside SpriteRendererComponent — updates its UV region each frame.
// Supports named clips, variable FPS per clip, looping, and one-shot animations.
//
// Usage:
//   auto* anim = node->AddComponent<AnimationComponent>();
//
//   // Add clip: name, sheet columns, sheet rows, start column, start row,
//   //           frame count, FPS, loop
//   anim->AddClip("idle",    4, 4, 0, 0, 4, 8.f,  true);
//   anim->AddClip("walk",    4, 4, 0, 1, 4, 12.f, true);
//   anim->AddClip("attack",  4, 4, 0, 2, 3, 18.f, false);
//   anim->AddClip("die",     4, 4, 0, 3, 4, 6.f,  false);
//
//   anim->Play("walk");

#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "../Component.h"
#include "SpriteRenderer.h"

namespace LightningEngine
{
	struct AnimationClip
	{
		std::string  name;
		int          sheetCols  = 1;  // total columns in the sprite sheet
		int          sheetRows  = 1;  // total rows
		int          startCol   = 0;  // first frame column
		int          startRow   = 0;  // row this clip lives on
		int          frameCount = 1;  // how many frames in the clip
		float        fps        = 12.f;
		bool         loop       = true;
	};

	class AnimationComponent : public Component
	{
	public:
		// Called when a non-looping clip finishes. Receives the clip name.
		std::function<void(const std::string&)> onClipEnd;

		// -----------------------------------------------------------------------
		// Clip management
		// -----------------------------------------------------------------------
		void AddClip(const char* name, int sheetCols, int sheetRows,
		             int startCol, int startRow, int frameCount,
		             float fps, bool loop = true)
		{
			AnimationClip c;
			c.name       = name;
			c.sheetCols  = sheetCols;
			c.sheetRows  = sheetRows;
			c.startCol   = startCol;
			c.startRow   = startRow;
			c.frameCount = frameCount;
			c.fps        = fps;
			c.loop       = loop;
			clips[name]  = c;
		}

		// Start playing a clip (resets frame if different clip).
		void Play(const std::string& name)
		{
			if (currentName == name && playing) return;
			auto it = clips.find(name);
			if (it == clips.end()) return;
			currentName  = name;
			currentFrame = 0;
			elapsed      = 0.f;
			playing      = true;
			finished     = false;
			applyFrame();
		}

		void Stop()  { playing = false; }
		void Resume(){ playing = true;  }

		bool IsPlaying()  const { return playing;  }
		bool IsFinished() const { return finished; }
		const std::string& CurrentClip() const { return currentName; }
		int  CurrentFrame() const { return currentFrame; }

		// -----------------------------------------------------------------------
		void Update(float dt) override
		{
			if (!playing || currentName.empty()) return;
			auto it = clips.find(currentName);
			if (it == clips.end()) return;

			const AnimationClip& clip = it->second;
			if (clip.fps <= 0.f || clip.frameCount <= 0) return;

			elapsed += dt / 1000.f;
			float frameDuration = 1.f / clip.fps;

			while (elapsed >= frameDuration)
			{
				elapsed -= frameDuration;
				currentFrame++;

				if (currentFrame >= clip.frameCount)
				{
					if (clip.loop)
					{
						currentFrame = 0;
					}
					else
					{
						currentFrame = clip.frameCount - 1;
						playing      = false;
						finished     = true;
						if (onClipEnd) onClipEnd(currentName);
						break;
					}
				}
			}

			applyFrame();
		}

	private:
		std::unordered_map<std::string, AnimationClip> clips;
		std::string currentName;
		int         currentFrame = 0;
		float       elapsed      = 0.f;
		bool        playing      = false;
		bool        finished     = false;

		void applyFrame()
		{
			if (!owner) return;
			auto it = clips.find(currentName);
			if (it == clips.end()) return;

			const AnimationClip& clip = it->second;
			auto* sr = owner->GetComponent<SpriteRendererComponent>();
			if (!sr) return;

			int col = clip.startCol + currentFrame;
			int row = clip.startRow;
			sr->SetSheetFrame(col, row, clip.sheetCols, clip.sheetRows);
		}
	};

} // namespace LightningEngine
