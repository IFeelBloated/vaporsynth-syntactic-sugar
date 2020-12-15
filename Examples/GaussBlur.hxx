#pragma once
#include "Interface.vxx"

struct GaussBlur {
	field(InputClip, VideoNode{});

public:
	static constexpr auto Name = "GaussBlur";
	static constexpr auto Signature = "clip:clip;";
	
public:
	GaussBlur(auto Arguments) {
		InputClip = Arguments["clip"];
		if (!InputClip.WithConstantFormat() || !InputClip.WithConstantDimensions() || !InputClip.IsSinglePrecision())
			throw RuntimeError{ "only single precision floating point clips with constant format and dimensions supported." };
	}
	auto RegisterMetadata(auto Core) {
		return InputClip.ExtractMetadata();
	}
	auto RequestReferenceFrames(auto Index, auto FrameContext) {
		InputClip.RequestFrame(Index, FrameContext);
	}
	auto DrawFrame(auto Index, auto Core, auto FrameContext) {
		auto InputFrame = InputClip.FetchFrame<const float>(Index, FrameContext);
		auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrame);
		auto GaussKernel = [](auto Center) {
			auto Conv = Center[-1][-1] + Center[-1][0] * 2 + Center[-1][1] +
				Center[0][-1] * 2 + Center[0][0] * 4 + Center[0][1] * 2 +
				Center[1][-1] + Center[1][0] * 2 + Center[1][1];
			return Conv / 16;
		};
		for (auto c : Range{ InputFrame.PlaneCount })
			for (auto y : Range{ InputFrame[c].Height })
				for (auto x : Range{ InputFrame[c].Width })
					ProcessedFrame[c][y][x] = GaussKernel(InputFrame[c].View(y, x));
		return ProcessedFrame.Leak();
	}
};