PGSuper/PGSplice Rating Options {#appendix_a_pgs_rating_options}
============
XBRate adds a tab titled XBeam to the Load Rating Options window in PGSuper and PGSplice. Load rating options specific to cross beams are defined on this tab.

Item | Description
-----|-----------
When PGSuper is in Envelope Mode, use | Select the analysis model to use for pier reactions when the Structural Analysis Method in PGSuper is set to "Envelope of Simple Span and Simple made Continuous Analysis". <table><tr><th>Item</th><th>Description</th></tr><tr><td>Simple</td><td>Reactions are based on simple span analysis</td></tr><tr><td>Continuous</td><td>Reactions are based on simple spans made continuous analysis</td></tr><tr><td>Envelope</td><td>Reactions are the greater of thos from simple span and a simple spans made continuous analysis</td></tr></table>
Compute emergency vehicle rating factors using | Select the method for computing emergency vehicle rating factors
Compute permit rating factors using | Select the method for computing permit rating factors
Maximum Live Load Step Size | Enter the maximum live load step size
Maximum Number of Lanes | Enter the maximum number of lanes to be considered in the analysis. Enter the keyword ALL to analyze for all lanes. Generally, controlling rating factors do not occur when all lanes are loaded. Using less than all of the lanes will decrease analysis time without impacting accuracy.
Analysis Locations for Negative Moment over Columns | By default, XBRate will perform negative cap moment rating calculations at faces of columns, column centerlines, and other interior column locations. However, computations in these interior location are not in the flexural "B" zone, possibly resulting in overly conservative, inaccurate results. Enable this option so that negative cap moment is analyzed at column face locations outward for columns having a greater or equal width than the specified value.

> NOTE: See Load Rating Options, @ref appendix_a_load_rating_options_analysis tab, for additional information on live load step size and number of lanes.

**Recalculate K using reduced rating demand (MBE C6A.5.6)** is a project option for Fourth Edition AASHTO ratings. New projects default to enabled; existing files without the setting load with it disabled. When enabled, eligible ratings below 1.0 recalculate K at reduced live load with permanent loads fixed. Turning it off retains K at the original loading. The setting is saved and participates in undo/redo. It does not change WSDOT permit/emergency methods. The details report identifies the selection. XBRate retains fixed beta/theta vertical shear; no uncracked MCFT or prestress size-effect checkbox is offered.
