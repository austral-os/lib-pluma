# Class `pluma::LayoutEngine`

**The engine responsible for computing geometry and constructing the Layout Tree.**

## Public Methods
- `LayoutEngine(std::shared_ptr< ITextShaper > shaper, std::shared_ptr< IFont > default_font)` - *Constructs a*
- `std::vector< std::unique_ptr< PageBox > > layoutText(std::string_view text, PageSize page_size, PageMargins margins, const FormatRegistry &registry, std::string_view header_text="", std::string_view footer_text="", uint32_t logical_offset_base=0)` - *Lays out a raw string into a series of paginated boxes.*

