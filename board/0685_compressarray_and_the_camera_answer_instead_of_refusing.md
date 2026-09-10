# 0685 CompressArray and the camera answer instead of refusing

**Two declared surfaces that had no behaviour, and both have one that is knowable.**

- **`System.CompressArray(Array of [Text])`**, 6 UT cases.
  `system-compressarray-method.md`: "Moves all non-empty strings (text) in an array to the
  beginning of the array. The resulting StringArray has the same number of elements as the input
  array, but empty entries appear at the end." So the length never changes, the order of the
  non-empty entries is kept, and the return is how many there are -- which is also where the empty
  ones begin. The gate case proves all three and its negative control was run: with the move
  neutered, 4 of its checks go red.
- **`CameraProvider` and `CameraOptions`**, 4 UT cases. A service tier has no client and therefore
  no camera, and `IsAvailable()` is exactly the question the `Camera` and `Media Upload` pages ask
  before they open. **It ANSWERS `false` rather than refusing**, because false is the true answer
  here; `RequestPictureAsync`, `EncodingType`, `AllowEdit`, `SourceType` and `MediaType` stay
  refusals, since nothing can take the picture.

**The rule this is a case of:** a refusal is right where the behaviour is unknown, and wrong where
the platform's answer on this deployment is knowable. `GuiAllowed()` is the same shape and already
answers.

**Measured.** Chain 102, A/B against chain 101.
