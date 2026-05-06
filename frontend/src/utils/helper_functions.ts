// use regex to make sure ceratin patterns are found
export function validatePostalCode(zip: string, country: string): boolean {
  const patterns: Record<string, RegExp> = {
    // regex patterns for specific countires where the pattern is known
    US: /^\d{5}(-\d{4})?$/,
    CA: /^[A-Za-z]\d[A-Za-z] ?\d[A-Za-z]\d$/,
    GB: /^[A-Za-z]{1,2}\d[A-Za-z\d]? ?\d[A-Za-z]{2}$/,
    AU: /^\d{4}$/,
    DE: /^\d{5}$/,
    FR: /^\d{5}$/,
    IT: /^\d{5}$/,
    ES: /^\d{5}$/,
    NL: /^\d{4} ?[A-Za-z]{2}$/,
    BE: /^\d{4}$/,
    DK: /^\d{4}$/,
    SE: /^\d{3} ?\d{2}$/,
    NO: /^\d{4}$/,
    FI: /^\d{5}$/,
    NZ: /^\d{4}$/,
    IE: /^[A-Za-z0-9 ]{3,7}$/,
    CH: /^\d{4}$/,
    AT: /^\d{4}$/,
    PT: /^\d{4}-\d{3}$/,
    GR: /^\d{3} ?\d{2}$/,
    PL: /^\d{2}-\d{3}$/,
    RU: /^\d{6}$/,
    IN: /^\d{6}$/,
    JP: /^\d{3}-\d{4}$/,
    CN: /^\d{6}$/,
    BR: /^\d{5}-?\d{3}$/,
    MX: /^\d{5}$/,
    ZA: /^\d{4}$/,
    KR: /^\d{5}$/,
    SG: /^\d{6}$/,
    AE: /^\d{5}$/,
    AR: /^\d{4}$/,
    CL: /^\d{7}$/,
  };

  // fallback: allow anything for all other countries
  const regex = patterns[country] || /.*/;
  return regex.test(zip);
}
