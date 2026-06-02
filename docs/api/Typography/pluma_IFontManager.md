# Class `pluma::IFontManager`

**Interface for querying and caching system fonts.**

## Public Methods
- `~IFontManager()=default`
- `std::shared_ptr< IFont > getFont(const FontDescriptor &desc)=0` - *Resolves a font descriptor to an actual loaded font.*

