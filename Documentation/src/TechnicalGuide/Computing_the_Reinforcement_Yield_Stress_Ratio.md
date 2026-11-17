Computing the Reinforcement Yield Check {#computing_the_reinforcement_yield_stress_ratio}
==============================================

AASHTO MBE 6A.5.4.2.2b provides an optional evaluation that limits reinforcement stress during permit rating. The usual allowable stress is 0.90 of yield strength. XBRate uses the cracked-section calculation illustrated in MBE A2A.13.2.2, with separate section properties and modular ratios for permanent and transient loads.

For Fourth Edition AASHTO permit ratings, the reports identify the existing calculation as a Service I rating factor:

`RF = (fr - fdl) / fll`

Here `fr` is allowable reinforcement stress, `fdl` is factored permanent-load stress, and `fll` is factored live-load stress at the specified permit weight, including applicable impact and multiple presence. Permanent-load stress stays fixed as the permit live load is multiplied by RF. A fixed-load stress failure gives RF = 0, including when the live-load moment or factor is zero. If fixed loads pass and there is no adverse live-load stress increment in the checked direction, the RF is unbounded.

The details also show the stress ratio at the specified truck weight, `fr / (fdl + fll)`. This diagnostic differs from the live-load RF. Both use the selected point and load case; the RF determines the controlling result. Earlier-edition report conventions remain unchanged.

When permit rating factors use the WSDOT BDM method, the arrangement of legal and permit vehicles that produces the governing permit moment rating factor is used for the reinforcement yield check. Its existing computation and reporting are retained. This method is separate from the refined AASHTO adjacent-legal-traffic equation in C6A.4.5.4.2c-1.

See [Computing Emergency Vehicle and Permit Rating Factors](@ref computing_permit_rating_factors) for more information.
