#
# Tag set for internal Explain-data management
#

name explain
type 4
include tagsetm.tag

#
# Explain categories
#
tag 0		targetInfo					structured
tag 1		databaseInfo					structured
tag 2		schemaInfo					structured
tag 3		tagSetInfo					structured
tag 4		recordSyntaxInfo				structured
# ....

#
# TargetInfo
#
tag 102		name						string
tag 103		recentNews					string
tag 104		icon						structured
tag 105		namedResultSets					bool
tag 106		multipleDbSearch				bool
tag 107		maxResultSets					numeric
tag 108		maxResultSize					numeric
tag 109		maxTerms					numeric
tag 110		timeoutInterval					intunit
tag 111		welcomeMessage					string
tag 112		contactInfo					structured
tag 113		description					string
tag 114		nicknames					structured
tag 115		usageRest					string
tag 116		paymentAddr					string
tag 117		hours						string
tag 118		dbCombinations					structured
tag 119		address						structured

#
# DatabaseInfo
#
tag 201		userFee						bool
tag 202		available					bool
tag 203		titleString					string
tag 205		associatedDbs					structured
tag 206		subDbs						structured
tag 207		disclaimers					string
tag 209		recordCount					structured
tag 210		recordCountActual				numeric
tag 211		recordCountApprox				numeric
tag 212		defaultOrder					string
tag 213		avRecordSize					numeric
tag 214		maxRecordSize					numeric
tag 215		hours						string
tag 216		bestTime					string
tag 217		lastUpdate					generalizedtime
tag 218		updateInterval					intunit
tag 219		coverage					string
tag 220		proprietary					bool
tag 221		copyrightText					string
tag 222		copyrightNotice					string
tag 223		producerContactInfo				structured
tag 224		supplierContactInfo				structured
tag 225		submissionContactInfo				structured
tag 226		explainDatabase					null
tag 227		keywords					string

#
# AccessInfo
#
tag 500		accessinfo					structured
tag 501		queryTypesSupported				structured
tag 503		diagnosticSets					structured
tag 505		attributeSetIds					structured
tag 507		schemas						structured
tag 509		recordSyntaxes					structured
tag 511		resourceChallenges				structured
tag 513		restrictedAccess				structured
tag 514		costInfo					structured
tag 515		variantSets					structured
tag 516		elementSetNames					structured
tag 517		unitSystems					structured

tag 600		commonInfo					structured
tag 601		dateAdded					generalizedtime
tag 602		dateChanged					generalizedtime
tag 603		expiry						generalizedtime
tag 604		languageCode					string

#
# General tags for list members, etc.
#
tag 1000	oid						oid
tag 1001	string						string
