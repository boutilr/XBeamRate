General {#appendix_a_load_rating_options_general}
============
General load rating options are defined on this tab.

## Specifications
Select the specifications upon which the load rating analysis will be based.

Item | Description
-----|----------------
AASHTO LRFD Bridge Design Specification | Select the AASHTO LRFD BDS that will be used for the load rating analysis
AASHTO Manual for Bridge Evaluation | Select the AASHTO MBE that will be used for the load rating analysis

## Design Rating (Inventory/Operating)
Item | Description
-----|------------
Perform load rating analysis | When checked, inventory and operating rating analysis is performed for flexure
Rate for Shear | When checked, inventory and operating rating analysis is performed for shear

## Legal Load Rating
Item | Description
-----|------------
Perform load rating analysis | When checked, legal load rating analysis is performed for flexure
Rate for Shear | When checked, legal load rating analysis is performed for shear
Compute emergency vehicle rating factors using | Select the option for computing the emergency vehicle rating factors.

## Permit Load Rating
Item | Description
-----|------------
Perform load rating analysis | When checked, permit load rating analysis is performed for flexure
Rate for Shear | When checked, permit load rating analysis is performed for shear
Check reinforcement yielding | When checked, reinforcement yielding is check according to MBE 6A.5.4.2.2b
Allowable Stress | Enter the allowable yield stress coefficient for the reinforcement yielding check
Compute permit rating factors using | Select the option for computing the permit rating factors.

**Recalculate K using reduced rating demand (MBE C6A.5.6)** is a project option for Fourth Edition AASHTO ratings. New projects default to enabled; existing files without the setting load with it disabled. When enabled, eligible ratings below 1.0 recalculate K at reduced live load with permanent loads fixed. Turning it off retains K at the original loading. The setting is saved and participates in undo/redo. It does not change WSDOT permit/emergency methods. The details report identifies the selection. XBRate retains fixed beta/theta vertical shear; no uncracked MCFT or prestress size-effect checkbox is offered.
