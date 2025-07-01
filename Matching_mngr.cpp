#include "Matching_mngr.h"
#include <iostream>

// Implementation der Template-Klasse Matching_mngr liegt vollständig im Header,
// aber wir können hier spezifische Template-Instanziierungen vornehmen.

// Dieses explizite Instanziieren der Klasse für die bekannten Typen (laptop, storage_drive) 
// kann Kompilierungszeit reduzieren und Code-Größe optimieren.
template class Matching_mngr<laptop>;
template class Matching_mngr<storage_drive>;