import editTokens from “@amzn/meridian-tokens/base/icon/edit”;
import Box from “@amzn/meridian/box”;
import Button from “@amzn/meridian/button”;
import Column from “@amzn/meridian/column”;
import Expander from “@amzn/meridian/expander”;
import Heading from “@amzn/meridian/heading”;
import Icon from “@amzn/meridian/icon”;
import Input from “@amzn/meridian/input”;
import Link from “@amzn/meridian/link”;
import Popover from “@amzn/meridian/popover”;
import Row from “@amzn/meridian/row”;
import Sheet from “@amzn/meridian/sheet”;
import Text from “@amzn/meridian/text”;
import { faPlus, faScissors } from “@fortawesome/free-solid-svg-icons”;
import { FontAwesomeIcon } from “@fortawesome/react-fontawesome”;
import Ansi from “ansi-to-react”;
import * as React from “react”;
import { useCallback, useEffect, useRef, useState } from “react”;
import { saveRecommendations } from “../../lib/data-storage/DynamoDBStore”;
import { versionSetName } from “../../lib/helpers/constants”;
import { reformatRecommendationFieldName } from “../../lib/helpers/parsing”;
import { defaultCutTicketUrl, getVersionSetDiffUrl } from “../../lib/helpers/urls”;
import { Recommendation } from “../../lib/recommendations/types”;
import { GenerateTestCaseButtonView } from “../generate-test-case/GenerateTestCaseView”;
import { CopyableText } from “./CopyableText”;
import { EvidenceCellContents } from “./EvidenceCellContents”;
import { cutTicketButtonOnClick, getConfidenceString } from “./common”;

type RecommendationSheetProps = {
    recommendation?: Recommendation;
    open: boolean;
    onClose?: () => void;
};

type RecommendationField = keyof Recommendation;

type OtherExpanderProps = {
    others: string[];
    title: string;
    open: boolean;
    setOpen: React.Dispatch<React.SetStateAction<boolean>>;
};

type TicketDetailsPopoverProps = {
    buttonRef: any;
    open: boolean;
    setOpen: React.Dispatch<React.SetStateAction<boolean>>;
};

// fields to exclude from the first recommendation details box shown in the sheet:
const recommendationFieldsToExclude: RecommendationField[] = [
    “assignReasonMarkdown”,
    “cutTicketUrl”,
    “rowFields”,
    “evidenceFor”,
    “evidenceAgainst”,
    “message”,
    “recommendationId”,
    “ticketUrl”,
];

// styling values:
const sheetWidth = “800px”;
const sheetPadding = “400”;
const headerPadding = “300 none 400 none”;
const bodyWrapperPadding = “300”;
const bodyPadding = “none none 400 none”;
const bodyContentsPadding = “none 400 none none”;
const bodyContentsSpacing = “500";
const recDescriptorFieldNameWidth = “200px”;
const recDescriptorRowSpacing = “400";
const recDescriptorBoxPadding = “400”;
const evidenceBoxPadding = “400";
const endpoint = “api/recommendation”;

export const RecommendationSheet = (props: RecommendationSheetProps) => {
    const { recommendation, open, onClose } = props;

    const [currentRecommendation, setCurrentRecommendation] = useState<Recommendation | undefined>(undefined);

    const [otherDeviceTypesOpen, setOtherDeviceTypesOpen] = useState<boolean>(false);
    const [otherTestsOpen, setOtherTestsOpen] = useState<boolean>(false);

    // The currently selected recommendation should always be defined, so that the sheet can animate smoothly when closed.
    // This maintains a pointer to the most recently chosen recommendation.
    useEffect(() => {
        if (recommendation) {
            setCurrentRecommendation(recommendation);
        }
    }, [recommendation]);

    const getRecommendationFieldContentsList = (values: Array<any>) => {
        return (
            <ol style={{ padding: “0 0 0 16px” }}>
                {values.map((value, i) => (
                    <li style={{ padding: “0 0 0 14px” }} key={i}>
                        <Row spacing=“300">
                            <CopyableText text={value.toString()} />
                        </Row>
                    </li>
                ))}
            </ol>
        );
    };

    const getRecommendationFieldContents = (fieldName: string, value: any) => {
        let contents = value;

        if (fieldName === “eventId”) {
            return <CopyableText isLink={true} href={getVersionSetDiffUrl(versionSetName, value)} text={value} />;
        }

        if (fieldName === “confidence”) {
            contents = getConfidenceString(value);
        }

        if (fieldName === “error”) {
            return (
                <Text>
                    <Ansi>{contents.toString()}</Ansi>
                </Text>
            );
        }

        if (fieldName === “affectedDeviceTypes”) {
            let first;
            if (value.directly.length === 0) {
                contents = “No devices linked to this recommendation affected”;
            } else {
                if (value.directly.length === 1) {
                    contents = value.directly[0];
                } else {
                    first = getRecommendationFieldContentsList(value.directly);
                }
            }

            if (contents !== value) first = <CopyableText text={contents.toString()} />;
            if (value.indirectly.length === 0) return first;

            return (
                <Column>
                    {first}
                    {OtherDevicesTestsExpander({
                        others: value.indirectly,
                        title: `${value.indirectly.length} other device(s) affected by the same issue`,
                        open: otherDeviceTypesOpen,
                        setOpen: setOtherDeviceTypesOpen,
                    })}
                </Column>
            );
        }

        if (fieldName === “affectedTests”) {
            let first;
            if (value.directly.length === 0) {
                contents = “No tests linked to this recommendation affected”;
            } else {
                if (value.directly.length === 1) {
                    contents = value.directly[0];
                } else {
                    first = getRecommendationFieldContentsList(value.directly);
                }
            }

            if (contents !== value) first = <CopyableText text={contents.toString()} />;
            if (value.indirectly.length === 0) return first;

            return (
                <Column>
                    {first}
                    {OtherDevicesTestsExpander({
                        others: value.indirectly,
                        title: `${value.indirectly.length} other test(s) affected by the same issue`,
                        open: otherTestsOpen,
                        setOpen: setOtherTestsOpen,
                    })}
                </Column>
            );
        }

        if (Array.isArray(value)) {
            if (value.length === 0) {
                contents = “”;
            } else if (value.length === 1) {
                contents = value[0];
            } else {
                return getRecommendationFieldContentsList(value);
            }
        }

        return <CopyableText text={contents.toString()} />;
    };

    const OtherDevicesTestsExpander = (props: OtherExpanderProps) => {
        return (
            <Expander open={props.open} onChange={props.setOpen} title={props.title} type=“box”>
                {getRecommendationFieldContentsList(props.others)}
            </Expander>
        );
    };

    const getRecommendationDescriptorRow = (fieldName: RecommendationField, key?: string | number) =>
        currentRecommendation && (
            <Row widths={[“fix”, “fill”]} key={key} alignmentVertical=“top”>
                <Column width={recDescriptorFieldNameWidth}>
                    <Text>
                        <strong>{reformatRecommendationFieldName(fieldName)}</strong>
                    </Text>
                </Column>
                <Column>{getRecommendationFieldContents(fieldName, currentRecommendation[fieldName])}</Column>
            </Row>
        );

    const TicketDetailsPopover = (props: TicketDetailsPopoverProps) => {
        const { buttonRef, open, setOpen } = props;
        const [value, setValue] = useState(currentRecommendation?.ticketUrl ?? “temp”);

        const onClose = useCallback(() => {
            setOpen(false);
        }, []);

        const onSubmit = useCallback(async () => {
            setOpen(false);
            if (currentRecommendation) {
                currentRecommendation.ticketUrl = value;
                const params = {
                    recommendationId: currentRecommendation.recommendationId,
                    updateParams: new Map<string, string>([[“ticketUrl”, currentRecommendation.ticketUrl]]),
                };
                const response = await fetch(endpoint, {
                    method: “POST”,
                    body: JSON.stringify(params),
                    headers: {
                        “Content-Type”: “application/json”,
                    },
                });
            }
        }, [value]);

        return (
            <Popover anchorNode={buttonRef.current} open={open} onClose={onClose} position=“bottom”>
                <Column>
                    <Input value={value} onChange={setValue} type=“text” />
                    <Button type=“tertiary” onClick={onSubmit}>
                        Submit
                    </Button>
                </Column>
            </Popover>
        );
    };

    const TicketDetails = () => {
        const buttonRef = useRef();
        const onClickButton = useCallback(() => setOpen(true), []);
        const [open, setOpen] = useState(false);

        if (!currentRecommendation?.ticketUrl) {
            return <></>;
        }

        return (
            <>
                <Row widths={[“fix”, “fill”]} key={“ticket”} alignmentVertical=“center”>
                    <Column width={recDescriptorFieldNameWidth}>
                        <Text>
                            <strong>{reformatRecommendationFieldName(“Ticket”)}</strong>
                        </Text>
                    </Column>
                    <Column>
                        <Row>
                            <Link href={currentRecommendation?.ticketUrl || “tempURL”} target=“_blank” key={“Ticket”}>
                                {currentRecommendation?.ticketUrl}
                            </Link>
                            <Button type=“icon” onClick={onClickButton} ref={buttonRef}>
                                <Icon tokens={editTokens} size={“xs”} />
                            </Button>
                        </Row>
                    </Column>
                </Row>
                <TicketDetailsPopover buttonRef={buttonRef} open={open} setOpen={setOpen} />
            </>
        );
    };

    const getRecommendationDescriptors = () => {
        if (!currentRecommendation) {
            return;
        }
        const orderedListOfFieldNames = (Object.keys(currentRecommendation) as RecommendationField[])
            .filter((field: RecommendationField) => !recommendationFieldsToExclude.includes(field))
            .sort();

        return (
            <Column type=“outline” spacingInset={recDescriptorBoxPadding} spacing={recDescriptorRowSpacing}>
                {orderedListOfFieldNames.map(getRecommendationDescriptorRow)}
                <TicketDetails />
            </Column>
        );
    };

    const SheetContents = () => {
        const buttonRef = useRef();
        const [open, setOpen] = useState(false);
        const onClickButton = useCallback(() => setOpen(true), []);

        if (!currentRecommendation) {
            return null;
        }

        const { message } = currentRecommendation;
        const cutTicketUrl = currentRecommendation.cutTicketUrl || defaultCutTicketUrl;

        const hasTicket = !!currentRecommendation.ticketUrl;

        return (
            <>
                <Box spacingInset={headerPadding} key=“header”>
                    <CopyableText
                        text={message}
                        tooltipText={“Copied hypothesis title”}
                        level={3}
                        type=“h500"
                        isHeading={true}
                    />
                </Box>
                <Box overflowY=“auto” key=“info” spacingInset={bodyPadding}>
                    <Box spacingInset={bodyContentsPadding}>
                        <Column spacing={bodyContentsSpacing}>
                            {getRecommendationDescriptors()}
                            <Column type=“outline” spacingInset={evidenceBoxPadding} spacing=“none”>
                                <Heading type=“h200” level={4}>
                                    Evidence for
                                </Heading>
                                <EvidenceCellContents evidenceList={currentRecommendation.evidenceFor} />
                            </Column>
                            <Column type=“outline” spacingInset={evidenceBoxPadding} spacing=“none”>
                                <Heading type=“h200" level={4}>
                                    Evidence against
                                </Heading>
                                <EvidenceCellContents evidenceList={currentRecommendation.evidenceAgainst} />
                            </Column>
                        </Column>
                    </Box>
                </Box>
                <Row>
                    {!hasTicket && (
                        <>
                            <Row width=“100%” key=“cut-tt-button”>
                                <Button
                                    onClick={onClickButton}
                                    href={“testurl”}
                                    target=“_blank”
                                    minWidth=“100%”
                                    ref={buttonRef}
                                >
                                    <FontAwesomeIcon icon={faPlus} />
                                    Add Ticket{” “}
                                </Button>
                                <TicketDetailsPopover buttonRef={buttonRef} open={open} setOpen={setOpen} />
                            </Row>
                        </>
                    )}
                    {cutTicketUrl && (
                        <Row width=“100%” key=“cut-tt-button”>
                            <Button
                                onClick={() => recommendation && cutTicketButtonOnClick(recommendation)}
                                href={cutTicketUrl}
                                target=“_blank”
                                minWidth=“100%”
                            >
                                <FontAwesomeIcon icon={faScissors} size={“sm”} />
                                Cut ticket{” “}
                            </Button>
                        </Row>
                    )}
                </Row>
                <Row width=“100%” key=“generate-test”>
                    <GenerateTestCaseButtonView contentAlignment=“center” recommendation={currentRecommendation} />
                </Row>
            </>
        );
    };

    return (
        <Sheet type=“overlay” side=“right” open={open} onClose={onClose} spacingInset={sheetPadding}>
            <Column height=“100%” width={sheetWidth} heights={[“fit”, “fill”, “fit”]} spacingInset={bodyWrapperPadding}>
                <SheetContents />
            </Column>
        </Sheet>
    );
};
